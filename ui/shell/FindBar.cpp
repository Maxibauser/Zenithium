#include "FindBar.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QShortcut>
#include <QTextCursor>
#include <QTextDocument>
#include <QToolButton>

namespace zen::ui {

FindBar::FindBar(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenFindBar");

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(8, 4, 8, 4);
    row->setSpacing(6);

    m_query = new QLineEdit(this);
    m_query->setObjectName("ZenFindInput");
    m_query->setPlaceholderText(tr("Find"));
    m_query->setClearButtonEnabled(true);
    m_query->setMinimumWidth(220);

    auto makeToolBtn = [this](const QString& text, const QString& tip, bool checkable = false) {
        auto* b = new QToolButton(this);
        b->setObjectName("ZenFindToolBtn");
        b->setText(text);
        b->setToolTip(tip);
        b->setAutoRaise(true);
        b->setCheckable(checkable);
        b->setCursor(Qt::PointingHandCursor);
        return b;
    };
    m_case  = makeToolBtn(QStringLiteral("Aa"), tr("Match case"),  true);
    m_word  = makeToolBtn(QStringLiteral("ab"), tr("Whole word"),  true);
    m_prev  = makeToolBtn(QStringLiteral("↑"),  tr("Previous (Shift+Enter)"));
    m_next  = makeToolBtn(QStringLiteral("↓"),  tr("Next (Enter)"));
    m_close = makeToolBtn(QStringLiteral("✕"),  tr("Close (Esc)"));

    m_status = new QLabel(QString(), this);
    m_status->setObjectName("ZenFindStatus");

    row->addWidget(m_query, 1);
    row->addWidget(m_case);
    row->addWidget(m_word);
    row->addWidget(m_prev);
    row->addWidget(m_next);
    row->addWidget(m_status);
    row->addStretch(1);
    row->addWidget(m_close);

    connect(m_query, &QLineEdit::returnPressed, this, &FindBar::findNext);
    connect(m_query, &QLineEdit::textEdited,    this, &FindBar::onTextEdited);
    connect(m_next,  &QToolButton::clicked,     this, &FindBar::findNext);
    connect(m_prev,  &QToolButton::clicked,     this, &FindBar::findPrev);
    connect(m_close, &QToolButton::clicked,     this, &FindBar::hideBar);

    // Shift+Enter for previous match while focused in the input.
    auto* prevSc = new QShortcut(QKeySequence(QStringLiteral("Shift+Return")), m_query);
    prevSc->setContext(Qt::WidgetShortcut);
    connect(prevSc, &QShortcut::activated, this, &FindBar::findPrev);

    auto* escSc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    escSc->setContext(Qt::WidgetWithChildrenShortcut);
    connect(escSc, &QShortcut::activated, this, &FindBar::hideBar);

    hide();
}

void FindBar::attachEditor(QPlainTextEdit* editor) {
    m_editor = editor;
}

void FindBar::showAndFocus() {
    // If there is a selection in the editor, pre-fill with it.
    if (m_editor) {
        const auto cur = m_editor->textCursor();
        if (cur.hasSelection()) {
            const QString sel = cur.selectedText();
            if (!sel.contains(QChar::ParagraphSeparator)) m_query->setText(sel);
        }
    }
    show();
    m_query->setFocus();
    m_query->selectAll();
}

void FindBar::hideBar() {
    hide();
    if (m_editor) m_editor->setFocus();
}

void FindBar::onTextEdited(const QString& text) {
    if (!m_editor || text.isEmpty()) { m_status->clear(); return; }
    // Search from the start of the current selection so the same match stays highlighted.
    auto cur = m_editor->textCursor();
    cur.setPosition(cur.selectionStart());
    m_editor->setTextCursor(cur);
    doFind(true);
}

void FindBar::findNext() { doFind(true); }
void FindBar::findPrev() { doFind(false); }

void FindBar::doFind(bool forward) {
    if (!m_editor) return;
    const QString needle = m_query->text();
    if (needle.isEmpty()) { m_status->clear(); return; }

    QTextDocument::FindFlags flags;
    if (!forward)          flags |= QTextDocument::FindBackward;
    if (m_case->isChecked()) flags |= QTextDocument::FindCaseSensitively;
    if (m_word->isChecked()) flags |= QTextDocument::FindWholeWords;

    if (m_editor->find(needle, flags)) {
        m_status->clear();
        return;
    }
    // Wrap around.
    auto cur = m_editor->textCursor();
    cur.movePosition(forward ? QTextCursor::Start : QTextCursor::End);
    m_editor->setTextCursor(cur);
    if (m_editor->find(needle, flags)) {
        m_status->setText(tr("wrapped"));
    } else {
        m_status->setText(tr("no results"));
    }
}

} // namespace zen::ui
