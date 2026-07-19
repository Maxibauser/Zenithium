#include "EditorView.h"

#include "Gutter.h"

#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>

#include "zen/document/Document.h"
#include "zen/document/LineDiff.h"

namespace zen::ui {

namespace {
constexpr int kGutterPadding    = 16;
constexpr int kChangeStripWidth = 4;
constexpr int kChangeStripGap   = 6;
} // namespace

EditorView::EditorView(QWidget* parent) : QPlainTextEdit(parent) {
    setObjectName("ZenEditorView");
    setFrameShape(QFrame::NoFrame);
    setLineWrapMode(QPlainTextEdit::NoWrap);
    setTabStopDistance(4 * fontMetrics().horizontalAdvance(QLatin1Char(' ')));

    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(11);
    mono.setStyleStrategy(QFont::PreferAntialias);
    setFont(mono);
    setTabStopDistance(4 * QFontMetrics(mono).horizontalAdvance(QLatin1Char(' ')));

    m_gutter = new Gutter(this);

    connect(this, &EditorView::blockCountChanged,   this, &EditorView::updateGutterWidth);
    connect(this, &EditorView::updateRequest,       this, &EditorView::updateGutter);
    connect(this, &EditorView::cursorPositionChanged, this, &EditorView::highlightCurrentLine);

    updateGutterWidth();
    highlightCurrentLine();
}

void EditorView::attachDocument(zen::document::Document* doc) {
    if (m_doc == doc) return;
    if (m_doc) {
        disconnect(m_doc, nullptr, this, nullptr);
    }
    m_doc = doc;
    if (!m_doc) {
        clear();
        return;
    }

    m_syncingFromDoc = true;
    setPlainText(m_doc->text());
    m_syncingFromDoc = false;

    connect(m_doc, &zen::document::Document::textChanged, this, [this] {
        if (!m_doc) return;
        if (toPlainText() != m_doc->text()) {
            m_syncingFromDoc = true;
            setPlainText(m_doc->text());
            m_syncingFromDoc = false;
        }
        recomputeDiff();
    });
    connect(m_doc, &zen::document::Document::savedTextChanged, this,
            &EditorView::recomputeDiff);

    connect(this, &QPlainTextEdit::textChanged, this, [this] {
        if (!m_doc || m_syncingFromDoc) return;
        m_doc->setText(toPlainText());
    });

    recomputeDiff();
}

void EditorView::recomputeDiff() {
    if (m_doc) {
        m_diff = zen::document::diffLines(m_doc->savedText(), toPlainText());
    } else {
        m_diff.clear();
    }
    m_gutter->update();
}

int EditorView::gutterWidth() const {
    int width = 0;
    if (m_showLineNumbers) {
        int digits = 1;
        int lines = qMax(1, blockCount());
        while (lines >= 10) { lines /= 10; ++digits; }
        const int advance = fontMetrics().horizontalAdvance(QLatin1Char('9'));
        width += kGutterPadding + advance * digits + kGutterPadding / 2;
    } else {
        width += kGutterPadding / 2;
    }
    if (m_showChangeBars) {
        width += kChangeStripGap + kChangeStripWidth;
    }
    return width;
}

void EditorView::setLineNumbersVisible(bool on) {
    if (m_showLineNumbers == on) return;
    m_showLineNumbers = on;
    updateGutterWidth();
    m_gutter->update();
}

void EditorView::setChangeBarsVisible(bool on) {
    if (m_showChangeBars == on) return;
    m_showChangeBars = on;
    updateGutterWidth();
    m_gutter->update();
}

void EditorView::updateGutterWidth() {
    setViewportMargins(gutterWidth(), 0, 0, 0);
}

void EditorView::updateGutter(const QRect& rect, int dy) {
    if (dy) {
        m_gutter->scroll(0, dy);
    } else {
        m_gutter->update(0, rect.y(), m_gutter->width(), rect.height());
    }
    if (rect.contains(viewport()->rect())) {
        updateGutterWidth();
    }
}

void EditorView::resizeEvent(QResizeEvent* event) {
    QPlainTextEdit::resizeEvent(event);
    const QRect cr = contentsRect();
    m_gutter->setGeometry(QRect(cr.left(), cr.top(), gutterWidth(), cr.height()));
}

void EditorView::highlightCurrentLine() {
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly()) {
        QTextEdit::ExtraSelection sel;
        sel.format.setBackground(QColor(0x1c, 0x21, 0x30));
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);
        sel.cursor = textCursor();
        sel.cursor.clearSelection();
        selections.append(sel);
    }
    setExtraSelections(selections);
}

void EditorView::paintGutter(QPaintEvent* event) {
    QPainter p(m_gutter);
    p.fillRect(event->rect(), QColor(0x12, 0x15, 0x1d));

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    const int currentLine = textCursor().blockNumber();
    const int stripRight  = m_gutter->width();
    const int stripLeft   = m_showChangeBars ? stripRight - kChangeStripWidth : stripRight;
    const int numberRight = m_showChangeBars ? stripLeft - kChangeStripGap    : stripRight;

    const QColor addedColor   (0x2e, 0xa0, 0x43);
    const QColor modifiedColor(0xd1, 0x9a, 0x66);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            if (m_showLineNumbers) {
                const bool isCurrent = (blockNumber == currentLine);
                p.setPen(isCurrent ? QColor(0xe6, 0xe9, 0xf2) : QColor(0x4a, 0x53, 0x6b));
                const QString number = QString::number(blockNumber + 1);
                p.drawText(0, top, numberRight, fontMetrics().height(),
                           Qt::AlignRight | Qt::AlignVCenter, number);
            }
            if (m_showChangeBars && blockNumber < m_diff.size()) {
                const auto k = m_diff[blockNumber];
                if (k != zen::document::LineChange::Unchanged) {
                    p.fillRect(stripLeft, top, kChangeStripWidth,
                               qMax(1, bottom - top),
                               k == zen::document::LineChange::Added
                                   ? addedColor : modifiedColor);
                }
            }
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

} // namespace zen::ui
