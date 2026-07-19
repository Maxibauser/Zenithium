#include "GitPanel.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>

namespace zen::ui {

// ----- Activity card ---------------------------------------------------------

class GitActivityCard : public QFrame {
    Q_OBJECT
public:
    explicit GitActivityCard(const QString& command, QWidget* parent = nullptr)
        : QFrame(parent) {
        setObjectName("ZenActivityCard");
        setProperty("state", "running");

        auto* v = new QVBoxLayout(this);
        v->setContentsMargins(10, 8, 10, 8);
        v->setSpacing(4);

        auto* header = new QHBoxLayout();
        header->setSpacing(8);

        m_cmd = new QLabel(command, this);
        m_cmd->setObjectName("ZenActivityCmd");
        m_cmd->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_cmd->setWordWrap(true);
        header->addWidget(m_cmd, 1);

        m_status = new QLabel(tr("running…"), this);
        m_status->setObjectName("ZenActivityStatus");
        header->addWidget(m_status, 0, Qt::AlignRight | Qt::AlignTop);

        v->addLayout(header);

        m_body = new QLabel(this);
        m_body->setObjectName("ZenActivityBody");
        m_body->setTextInteractionFlags(Qt::TextSelectableByMouse);
        m_body->setWordWrap(true);
        m_body->setTextFormat(Qt::PlainText);
        m_body->hide();
        v->addWidget(m_body);
    }

    void appendLine(const QString& line) {
        if (!m_bodyText.isEmpty()) m_bodyText += QLatin1Char('\n');
        m_bodyText += line;
        m_body->setText(m_bodyText);
        m_body->show();
    }

    void markDone(int exitCode) {
        if (exitCode == 0) {
            setProperty("state", "ok");
            m_status->setText(QStringLiteral("✓ done"));
        } else {
            setProperty("state", "fail");
            m_status->setText(QStringLiteral("✗ exit %1").arg(exitCode));
        }
        // Trigger QSS re-eval so the property-based colors apply.
        style()->unpolish(this);
        style()->polish(this);
        style()->unpolish(m_status);
        style()->polish(m_status);
    }

private:
    QLabel* m_cmd    {nullptr};
    QLabel* m_status {nullptr};
    QLabel* m_body   {nullptr};
    QString m_bodyText;
};

// ----- Helpers ---------------------------------------------------------------

namespace {

QString statusBadge(zen::git::FileStatus s) {
    using S = zen::git::FileStatus;
    switch (s) {
        case S::Modified:    return QStringLiteral("M");
        case S::Added:       return QStringLiteral("A");
        case S::Deleted:     return QStringLiteral("D");
        case S::Renamed:     return QStringLiteral("R");
        case S::Copied:      return QStringLiteral("C");
        case S::Untracked:   return QStringLiteral("U");
        case S::Ignored:     return QStringLiteral("I");
        case S::Conflicted:  return QStringLiteral("!");
        case S::TypeChanged: return QStringLiteral("T");
        default:             return QStringLiteral(" ");
    }
}

QColor statusColor(zen::git::FileStatus s) {
    using S = zen::git::FileStatus;
    switch (s) {
        case S::Modified:   return QColor("#e0af68");
        case S::Added:      return QColor("#9ece6a");
        case S::Deleted:    return QColor("#f7768e");
        case S::Untracked:  return QColor("#7aa2f7");
        case S::Renamed:    return QColor("#bb9af7");
        case S::Conflicted: return QColor("#f7768e");
        default:            return QColor("#8b93a7");
    }
}

QString shortName(const QString& path) {
    const int slash = path.lastIndexOf(QLatin1Char('/'));
    return slash < 0 ? path : path.mid(slash + 1);
}
QString dirName(const QString& path) {
    const int slash = path.lastIndexOf(QLatin1Char('/'));
    return slash < 0 ? QString() : path.left(slash);
}

} // namespace

// ----- GitPanel --------------------------------------------------------------

GitPanel::GitPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenGitPanel");
    m_git = new zen::git::GitService(this);
    buildUi();
    wireActions();
}

void GitPanel::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);

    // -- Header row --
    auto* header = new QHBoxLayout();
    auto* title = new QLabel(tr("SOURCE CONTROL"), this);
    title->setObjectName("ZenGitHeader");
    header->addWidget(title);
    header->addStretch(1);

    auto makeIconBtn = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton(this);
        b->setObjectName("ZenGitToolBtn");
        b->setText(text);
        b->setToolTip(tip);
        b->setAutoRaise(true);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };
    m_refresh = makeIconBtn(QStringLiteral("↻"), tr("Refresh status"));
    m_fetch   = makeIconBtn(QStringLiteral("⤓"), tr("Fetch"));
    m_pull    = makeIconBtn(QStringLiteral("↧"), tr("Pull"));
    m_push    = makeIconBtn(QStringLiteral("↥"), tr("Push"));
    header->addWidget(m_refresh);
    header->addWidget(m_fetch);
    header->addWidget(m_pull);
    header->addWidget(m_push);
    root->addLayout(header);

    // -- Branch card --
    auto* branchCard = new QFrame(this);
    branchCard->setObjectName("ZenBranchCard");
    auto* bcLayout = new QVBoxLayout(branchCard);
    bcLayout->setContentsMargins(12, 10, 12, 10);
    bcLayout->setSpacing(2);

    auto* branchRow = new QHBoxLayout();
    m_branch = new QLabel(tr("(no repository)"), branchCard);
    m_branch->setObjectName("ZenGitBranch");
    m_ahead  = new QLabel(QString(), branchCard);
    m_ahead->setObjectName("ZenGitAhead");
    branchRow->addWidget(m_branch);
    branchRow->addStretch(1);
    branchRow->addWidget(m_ahead);
    bcLayout->addLayout(branchRow);

    m_upstream = new QLabel(QString(), branchCard);
    m_upstream->setObjectName("ZenGitUpstream");
    m_upstream->hide();
    bcLayout->addWidget(m_upstream);
    root->addWidget(branchCard);

    // Hint / init / publish (shown situationally)
    m_repoHint = new QLabel(tr("This folder is not a git repository."), this);
    m_repoHint->setObjectName("ZenGitHint");
    m_repoHint->setWordWrap(true);
    root->addWidget(m_repoHint);

    m_init = new QPushButton(tr("Initialize Repository"), this);
    m_init->setObjectName("ZenGitInit");
    m_init->hide();
    root->addWidget(m_init);

    m_publish = new QPushButton(tr("Publish Branch to Remote…"), this);
    m_publish->setObjectName("ZenGitInit");
    m_publish->hide();
    root->addWidget(m_publish);

    // -- Commit box --
    m_message = new QPlainTextEdit(this);
    m_message->setObjectName("ZenGitMessage");
    m_message->setPlaceholderText(tr("Commit message…"));
    m_message->setFixedHeight(72);
    root->addWidget(m_message);

    auto* actionRow = new QHBoxLayout();
    m_commit   = new QPushButton(tr("✓  Commit"), this);
    m_commit->setObjectName("ZenGitCommit");
    m_stageAll = new QPushButton(tr("+  Stage All"), this);
    m_stageAll->setObjectName("ZenGitStageAll");
    actionRow->addWidget(m_commit, 1);
    actionRow->addWidget(m_stageAll);
    root->addLayout(actionRow);

    // -- Staged --
    m_stagedHeader = new QLabel(tr("STAGED CHANGES"), this);
    m_stagedHeader->setObjectName("ZenGitSectionLbl");
    root->addWidget(m_stagedHeader);
    m_staged = new QListWidget(this);
    m_staged->setObjectName("ZenGitList");
    m_staged->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_staged->setUniformItemSizes(true);
    m_staged->setToolTip(tr("Double-click to unstage"));
    root->addWidget(m_staged, 1);

    // -- Unstaged --
    m_changesHeader = new QLabel(tr("CHANGES"), this);
    m_changesHeader->setObjectName("ZenGitSectionLbl");
    root->addWidget(m_changesHeader);
    m_unstaged = new QListWidget(this);
    m_unstaged->setObjectName("ZenGitList");
    m_unstaged->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_unstaged->setUniformItemSizes(true);
    m_unstaged->setToolTip(tr("Double-click to stage"));
    root->addWidget(m_unstaged, 1);

    // -- Activity stream --
    auto* actHdr = new QLabel(tr("ACTIVITY"), this);
    actHdr->setObjectName("ZenGitSectionLbl");
    root->addWidget(actHdr);

    m_activityScroll = new QScrollArea(this);
    m_activityScroll->setObjectName("ZenActivityScroll");
    m_activityScroll->setWidgetResizable(true);
    m_activityScroll->setFrameShape(QFrame::NoFrame);
    m_activityScroll->setFixedHeight(180);

    m_activityHost = new QWidget(m_activityScroll);
    m_activityHost->setObjectName("ZenActivityHost");
    m_activityLayout = new QVBoxLayout(m_activityHost);
    m_activityLayout->setContentsMargins(0, 0, 0, 0);
    m_activityLayout->setSpacing(6);

    m_activityHint = new QLabel(tr("No recent git activity."), m_activityHost);
    m_activityHint->setObjectName("ZenGitHint");
    m_activityLayout->addWidget(m_activityHint);
    m_activityLayout->addStretch(1);

    m_activityScroll->setWidget(m_activityHost);
    root->addWidget(m_activityScroll);
}

void GitPanel::wireActions() {
    connect(m_git, &zen::git::GitService::statusUpdated,
            this, &GitPanel::onStatusUpdated);
    connect(m_git, &zen::git::GitService::logLine,
            this, &GitPanel::onLogLine);
    connect(m_git, &zen::git::GitService::commandDone,
            this, &GitPanel::onCommandDone);

    connect(m_refresh, &QToolButton::clicked, m_git, &zen::git::GitService::refreshStatus);
    connect(m_fetch,   &QToolButton::clicked, m_git, &zen::git::GitService::fetch);
    connect(m_pull,    &QToolButton::clicked, m_git, &zen::git::GitService::pull);
    connect(m_push,    &QToolButton::clicked, m_git, &zen::git::GitService::push);

    connect(m_init, &QPushButton::clicked, m_git, &zen::git::GitService::initRepo);

    connect(m_publish, &QPushButton::clicked, this, [this] {
        if (m_currentBranch.isEmpty()) {
            pushActivityCard(QStringLiteral("git publish"));
            m_activeCard->appendLine(tr("no current branch — commit first"));
            m_activeCard->markDone(1);
            m_activeCard = nullptr;
            return;
        }
        bool ok = false;
        const QString url = QInputDialog::getText(
            this, tr("Publish Branch"),
            tr("Remote URL for origin (e.g. https://github.com/you/repo.git):"),
            QLineEdit::Normal, QString(), &ok);
        if (!ok || url.trimmed().isEmpty()) return;
        m_git->publishToRemote(url.trimmed(), m_currentBranch);
    });

    connect(m_stageAll, &QPushButton::clicked, m_git, &zen::git::GitService::stageAll);
    connect(m_commit,   &QPushButton::clicked, this, [this] {
        const QString msg = m_message->toPlainText();
        m_git->commit(msg);
        m_message->clear();
    });

    connect(m_staged, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        m_git->unstage(selectedPaths(m_staged));
    });
    connect(m_unstaged, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem*) {
        m_git->stage(selectedPaths(m_unstaged));
    });
}

QStringList GitPanel::selectedPaths(QListWidget* list) const {
    QStringList out;
    const auto items = list->selectedItems();
    for (auto* it : items) out << it->data(Qt::UserRole).toString();
    return out;
}

void GitPanel::addFileRow(QListWidget* list,
                          const zen::git::FileEntry& e,
                          bool staged) {
    const auto code = staged ? e.indexStatus : e.workingStatus;
    const QString badge = statusBadge(code);
    const QString name  = shortName(e.path);
    const QString dir   = dirName(e.path);

    // Row layout:  ●  M   filename.cpp                     path/to/
    QString text;
    if (dir.isEmpty())
        text = QStringLiteral("●  %1   %2").arg(badge, name);
    else
        text = QStringLiteral("●  %1   %2    %3").arg(badge, name, dir);

    auto* item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, e.path);
    item->setToolTip(e.path);
    item->setForeground(statusColor(code));
    list->addItem(item);
}

void GitPanel::setRepoFolder(const QString& folderPath) {
    m_git->setRepoPath(folderPath);
}

void GitPanel::onStatusUpdated(const zen::git::StatusSnapshot& snap) {
    m_staged  ->clear();
    m_unstaged->clear();

    if (!snap.isRepo) {
        m_branch->setText(tr("(no repository)"));
        m_ahead ->setText(QString());
        m_upstream->hide();
        const bool hasFolder = !m_git->requestedPath().isEmpty();
        m_repoHint->setText(hasFolder
            ? tr("This folder is not a git repository.")
            : tr("Open a folder to use source control."));
        m_repoHint->show();
        m_init->setVisible(hasFolder);
        m_publish->hide();
        m_commit  ->setEnabled(false);
        m_stageAll->setEnabled(false);
        m_push->setEnabled(false); m_pull->setEnabled(false);
        m_fetch->setEnabled(false);
        m_stagedHeader ->setText(tr("STAGED CHANGES"));
        m_changesHeader->setText(tr("CHANGES"));
        emit branchInfoChanged(QString(), 0, 0);
        return;
    }

    m_repoHint->hide();
    m_init->hide();
    m_currentBranch = snap.branch;
    m_publish->setVisible(snap.upstream.isEmpty() && !snap.branch.isEmpty());

    m_commit  ->setEnabled(true);
    m_stageAll->setEnabled(true);
    m_push->setEnabled(true); m_pull->setEnabled(true);
    m_fetch->setEnabled(true);

    m_branch->setText(QStringLiteral("⎇  %1").arg(
        snap.branch.isEmpty() ? tr("HEAD") : snap.branch));
    if (!snap.upstream.isEmpty()) {
        m_upstream->setText(QStringLiteral("→  %1").arg(snap.upstream));
        m_upstream->show();
    } else {
        m_upstream->hide();
    }
    QString ab;
    if (snap.ahead)  ab += QStringLiteral("↑%1 ").arg(snap.ahead);
    if (snap.behind) ab += QStringLiteral("↓%1").arg(snap.behind);
    m_ahead->setText(ab);

    int staged = 0, unstaged = 0;
    for (const auto& e : snap.entries) {
        if (e.indexStatus != zen::git::FileStatus::Unmodified
         && e.indexStatus != zen::git::FileStatus::Untracked) {
            addFileRow(m_staged, e, /*staged*/ true);
            ++staged;
        }
        if (e.workingStatus != zen::git::FileStatus::Unmodified) {
            addFileRow(m_unstaged, e, /*staged*/ false);
            ++unstaged;
        }
    }
    m_stagedHeader ->setText(tr("STAGED CHANGES  (%1)").arg(staged));
    m_changesHeader->setText(tr("CHANGES  (%1)").arg(unstaged));

    emit branchInfoChanged(snap.branch, snap.ahead, snap.behind);
}

void GitPanel::pushActivityCard(const QString& command) {
    if (m_activityHint) { m_activityHint->hide(); }
    auto* card = new GitActivityCard(command, m_activityHost);
    // Insert before the trailing stretch (last item).
    m_activityLayout->insertWidget(m_activityLayout->count() - 1, card);
    m_activeCard = card;

    // Auto-scroll to bottom.
    QMetaObject::invokeMethod(this, [this] {
        auto* vs = m_activityScroll->verticalScrollBar();
        vs->setValue(vs->maximum());
    }, Qt::QueuedConnection);
}

void GitPanel::onLogLine(const QString& line) {
    // Lines starting with "$ " are new command headers; anything else is
    // streamed into the currently active card.
    if (line.startsWith(QLatin1String("$ "))) {
        pushActivityCard(line.mid(2));
        return;
    }
    if (line.startsWith(QLatin1String("[git]"))) {
        // service-level notice — show as its own single-line card
        pushActivityCard(line);
        m_activeCard->markDone(0);
        m_activeCard = nullptr;
        return;
    }
    if (m_activeCard) m_activeCard->appendLine(line);
}

void GitPanel::onCommandDone(const QString& /*command*/, int exitCode) {
    if (m_activeCard) {
        m_activeCard->markDone(exitCode);
        m_activeCard = nullptr;
    }
    // Scroll to newest card.
    QMetaObject::invokeMethod(this, [this] {
        auto* vs = m_activityScroll->verticalScrollBar();
        vs->setValue(vs->maximum());
    }, Qt::QueuedConnection);
}

} // namespace zen::ui

#include "GitPanel.moc"
