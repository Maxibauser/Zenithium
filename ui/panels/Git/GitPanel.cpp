#include "GitPanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>

namespace zen::ui {

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
        default:             return QString();
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

} // namespace

GitPanel::GitPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenGitPanel");
    m_git = new zen::git::GitService(this);
    buildUi();
    wireActions();
}

void GitPanel::buildUi() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(10, 10, 10, 10);
    root->setSpacing(8);

    // Header row.
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

    // Branch line.
    auto* branchRow = new QHBoxLayout();
    m_branch  = new QLabel(tr("(no repository)"), this);
    m_branch->setObjectName("ZenGitBranch");
    m_ahead   = new QLabel(QString(), this);
    m_ahead->setObjectName("ZenGitAhead");
    branchRow->addWidget(m_branch);
    branchRow->addStretch(1);
    branchRow->addWidget(m_ahead);
    root->addLayout(branchRow);

    m_repoHint = new QLabel(
        tr("This folder is not a git repository."), this);
    m_repoHint->setObjectName("ZenGitHint");
    m_repoHint->setWordWrap(true);
    root->addWidget(m_repoHint);

    m_init = new QPushButton(tr("Initialize Repository"), this);
    m_init->setObjectName("ZenGitInit");
    m_init->hide();
    root->addWidget(m_init);

    // Commit message.
    m_message = new QPlainTextEdit(this);
    m_message->setObjectName("ZenGitMessage");
    m_message->setPlaceholderText(tr("Commit message (Ctrl+Enter to commit)"));
    m_message->setFixedHeight(72);
    root->addWidget(m_message);

    // Commit + stage-all row.
    auto* actionRow = new QHBoxLayout();
    m_commit   = new QPushButton(tr("Commit"), this);
    m_commit->setObjectName("ZenGitCommit");
    m_stageAll = new QPushButton(tr("Stage All"), this);
    m_stageAll->setObjectName("ZenGitStageAll");
    actionRow->addWidget(m_commit, 1);
    actionRow->addWidget(m_stageAll);
    root->addLayout(actionRow);

    // Staged.
    auto* stagedLbl = new QLabel(tr("STAGED"), this);
    stagedLbl->setObjectName("ZenGitSectionLbl");
    root->addWidget(stagedLbl);
    m_staged = new QListWidget(this);
    m_staged->setObjectName("ZenGitList");
    m_staged->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_staged->setUniformItemSizes(true);
    root->addWidget(m_staged, 1);

    // Unstaged.
    auto* changesLbl = new QLabel(tr("CHANGES"), this);
    changesLbl->setObjectName("ZenGitSectionLbl");
    root->addWidget(changesLbl);
    m_unstaged = new QListWidget(this);
    m_unstaged->setObjectName("ZenGitList");
    m_unstaged->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_unstaged->setUniformItemSizes(true);
    root->addWidget(m_unstaged, 1);

    // Output log.
    auto* outLbl = new QLabel(tr("OUTPUT"), this);
    outLbl->setObjectName("ZenGitSectionLbl");
    root->addWidget(outLbl);
    m_output = new QPlainTextEdit(this);
    m_output->setObjectName("ZenGitOutput");
    m_output->setReadOnly(true);
    m_output->setMaximumBlockCount(500);
    m_output->setFixedHeight(110);
    root->addWidget(m_output);
}

void GitPanel::wireActions() {
    connect(m_git, &zen::git::GitService::statusUpdated,
            this, &GitPanel::onStatusUpdated);
    connect(m_git, &zen::git::GitService::logLine,
            this, &GitPanel::onLogLine);

    connect(m_refresh, &QToolButton::clicked, m_git, &zen::git::GitService::refreshStatus);
    connect(m_fetch,   &QToolButton::clicked, m_git, &zen::git::GitService::fetch);
    connect(m_pull,    &QToolButton::clicked, m_git, &zen::git::GitService::pull);
    connect(m_push,    &QToolButton::clicked, m_git, &zen::git::GitService::push);

    connect(m_init, &QPushButton::clicked, m_git, &zen::git::GitService::initRepo);
    connect(m_stageAll, &QPushButton::clicked, m_git, &zen::git::GitService::stageAll);
    connect(m_commit,   &QPushButton::clicked, this, [this] {
        const QString msg = m_message->toPlainText();
        m_git->commit(msg);
        m_message->clear();
    });

    // Double-click staged -> unstage, double-click unstaged -> stage.
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
    auto* item = new QListWidgetItem(QStringLiteral("  %1   %2").arg(badge, e.path));
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
        const bool hasFolder = !m_git->requestedPath().isEmpty();
        m_repoHint->setText(hasFolder
            ? tr("This folder is not a git repository.")
            : tr("Open a folder to use source control."));
        m_repoHint->show();
        m_init->setVisible(hasFolder);
        m_commit  ->setEnabled(false);
        m_stageAll->setEnabled(false);
        m_push->setEnabled(false); m_pull->setEnabled(false);
        m_fetch->setEnabled(false); m_refresh->setEnabled(false);
        return;
    }

    m_repoHint->hide();
    m_init->hide();
    m_commit  ->setEnabled(true);
    m_stageAll->setEnabled(true);
    m_push->setEnabled(true); m_pull->setEnabled(true);
    m_fetch->setEnabled(true); m_refresh->setEnabled(true);

    m_branch->setText(QStringLiteral("⎇  %1").arg(snap.branch.isEmpty() ? tr("HEAD") : snap.branch));
    QString ab;
    if (snap.ahead)  ab += QStringLiteral("↑%1 ").arg(snap.ahead);
    if (snap.behind) ab += QStringLiteral("↓%1").arg(snap.behind);
    m_ahead->setText(ab);

    for (const auto& e : snap.entries) {
        if (e.indexStatus != zen::git::FileStatus::Unmodified
         && e.indexStatus != zen::git::FileStatus::Untracked) {
            addFileRow(m_staged, e, /*staged*/ true);
        }
        if (e.workingStatus != zen::git::FileStatus::Unmodified) {
            addFileRow(m_unstaged, e, /*staged*/ false);
        }
    }
}

void GitPanel::onLogLine(const QString& line) {
    m_output->appendPlainText(line);
}

} // namespace zen::ui
