#pragma once

#include <QPlainTextEdit>
#include <QVector>

#include "zen/document/LineDiff.h"

namespace zen::document { class Document; }

namespace zen::ui {

class Gutter;

class EditorView : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit EditorView(QWidget* parent = nullptr);

    void attachDocument(zen::document::Document* doc);

    int gutterWidth() const;
    void paintGutter(QPaintEvent* event);

    void setLineNumbersVisible(bool on);
    void setChangeBarsVisible(bool on);
    [[nodiscard]] bool lineNumbersVisible() const noexcept { return m_showLineNumbers; }
    [[nodiscard]] bool changeBarsVisible() const noexcept { return m_showChangeBars; }

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateGutterWidth();
    void updateGutter(const QRect& rect, int dy);
    void highlightCurrentLine();
    void recomputeDiff();

private:
    Gutter*  m_gutter {nullptr};
    zen::document::Document* m_doc {nullptr};
    bool     m_syncingFromDoc {false};
    bool     m_showLineNumbers {true};
    bool     m_showChangeBars  {true};
    QVector<zen::document::LineChange> m_diff;
};

} // namespace zen::ui
