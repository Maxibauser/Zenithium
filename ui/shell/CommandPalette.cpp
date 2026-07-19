#include "CommandPalette.h"

#include <QApplication>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

namespace zen::ui {

namespace {

// Very small subsequence-match scorer: returns true if all chars of needle
// appear in haystack in order (case-insensitive). Score = negative first-index
// so earlier matches rank higher.
bool subsequenceScore(const QString& hay, const QString& needle, int* score) {
    if (needle.isEmpty()) { *score = 0; return true; }
    int i = 0;
    int firstIdx = -1;
    for (int h = 0; h < hay.size() && i < needle.size(); ++h) {
        if (hay[h].toLower() == needle[i].toLower()) {
            if (firstIdx < 0) firstIdx = h;
            ++i;
        }
    }
    if (i < needle.size()) return false;
    *score = -firstIdx;
    return true;
}

} // namespace

CommandPalette::CommandPalette(QWidget* parent) : QFrame(parent) {
    setObjectName("ZenCommandPalette");
    setFrameShape(QFrame::StyledPanel);
    setWindowFlag(Qt::Popup);
    setFocusPolicy(Qt::StrongFocus);
    resize(560, 360);

    auto* v = new QVBoxLayout(this);
    v->setContentsMargins(10, 10, 10, 10);
    v->setSpacing(6);

    m_query = new QLineEdit(this);
    m_query->setObjectName("ZenCommandInput");
    m_query->setPlaceholderText(tr("Type a command…"));
    m_query->installEventFilter(this);
    v->addWidget(m_query);

    m_list = new QListWidget(this);
    m_list->setObjectName("ZenCommandList");
    m_list->setUniformItemSizes(true);
    v->addWidget(m_list, 1);

    connect(m_query, &QLineEdit::textChanged, this, &CommandPalette::onTextChanged);
    connect(m_query, &QLineEdit::returnPressed, this, &CommandPalette::runSelected);
    connect(m_list,  &QListWidget::itemActivated,
            this, [this](QListWidgetItem*) { runSelected(); });
}

void CommandPalette::setCommands(QVector<Command> commands) {
    m_all = std::move(commands);
}

void CommandPalette::showPalette() {
    m_query->clear();
    repopulate();

    // Center over parent (or screen).
    QWidget* anchor = parentWidget() ? parentWidget()->window() : nullptr;
    if (anchor) {
        const QRect g = anchor->geometry();
        move(g.center().x() - width() / 2, g.top() + 80);
    }
    show();
    raise();
    m_query->setFocus();
}

bool CommandPalette::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_query && event->type() == QEvent::KeyPress) {
        auto* ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Down) {
            const int row = qMin(m_list->currentRow() + 1, m_list->count() - 1);
            m_list->setCurrentRow(row);
            return true;
        }
        if (ke->key() == Qt::Key_Up) {
            const int row = qMax(m_list->currentRow() - 1, 0);
            m_list->setCurrentRow(row);
            return true;
        }
        if (ke->key() == Qt::Key_Escape) {
            hide();
            return true;
        }
    }
    return QFrame::eventFilter(watched, event);
}

void CommandPalette::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Escape) { hide(); return; }
    QFrame::keyPressEvent(e);
}

void CommandPalette::onTextChanged(const QString&) {
    repopulate();
}

void CommandPalette::repopulate() {
    const QString q = m_query->text().trimmed();
    m_list->clear();
    m_filtered.clear();

    struct Ranked { int idx; int score; };
    QVector<Ranked> ranked;

    for (int i = 0; i < m_all.size(); ++i) {
        const auto& c = m_all[i];
        int score = 0;
        const QString haystack = c.category.isEmpty()
            ? c.name
            : (c.category + QLatin1String(": ") + c.name);
        if (subsequenceScore(haystack, q, &score)) {
            ranked.push_back({i, score});
        }
    }
    std::sort(ranked.begin(), ranked.end(),
              [](const Ranked& a, const Ranked& b) { return a.score > b.score; });

    for (const auto& r : ranked) {
        const auto& c = m_all[r.idx];
        QString label = c.category.isEmpty()
            ? c.name
            : QStringLiteral("%1  •  %2").arg(c.category, c.name);
        if (!c.shortcut.isEmpty()) {
            label += QStringLiteral("    (%1)").arg(c.shortcut);
        }
        m_list->addItem(label);
        m_filtered.push_back(r.idx);
    }
    if (m_list->count() > 0) m_list->setCurrentRow(0);
}

void CommandPalette::runSelected() {
    const int row = m_list->currentRow();
    if (row < 0 || row >= m_filtered.size()) return;
    const int idx = m_filtered[row];
    hide();
    if (m_all[idx].run) m_all[idx].run();
}

} // namespace zen::ui
