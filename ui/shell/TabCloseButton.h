#pragma once

#include <QToolButton>

namespace zen::ui {

// A small × button installed on each tab via QTabBar::setTabButton.
// Draws itself (no icon file needed) with a subtle rounded hover state.
class TabCloseButton : public QToolButton {
    Q_OBJECT
public:
    explicit TabCloseButton(QWidget* parent = nullptr);

    QSize sizeHint() const override { return {18, 18}; }

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
};

} // namespace zen::ui
