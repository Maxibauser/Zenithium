#include "Gutter.h"

#include "EditorView.h"

namespace zen::ui {

Gutter::Gutter(EditorView* editor) : QWidget(editor), m_editor(editor) {
    setObjectName("ZenGutter");
}

QSize Gutter::sizeHint() const {
    return QSize(m_editor->gutterWidth(), 0);
}

void Gutter::paintEvent(QPaintEvent* event) {
    m_editor->paintGutter(event);
}

} // namespace zen::ui
