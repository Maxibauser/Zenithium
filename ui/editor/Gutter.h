#pragma once

#include <QWidget>

namespace zen::ui {

class EditorView;

class Gutter : public QWidget {
    Q_OBJECT
public:
    explicit Gutter(EditorView* editor);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    EditorView* m_editor {nullptr};
};

} // namespace zen::ui
