#include "TabCloseButton.h"

#include <QPainter>

namespace zen::ui {

TabCloseButton::TabCloseButton(QWidget* parent) : QToolButton(parent) {
    setObjectName("ZenTabClose");
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    setAutoRaise(true);
    setFixedSize(18, 18);
}

void TabCloseButton::enterEvent(QEnterEvent* event) {
    QToolButton::enterEvent(event);
    update();
}

void TabCloseButton::leaveEvent(QEvent* event) {
    QToolButton::leaveEvent(event);
    update();
}

void TabCloseButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRectF r = rect().adjusted(1, 1, -1, -1);

    if (underMouse() || isDown()) {
        p.setPen(Qt::NoPen);
        p.setBrush(isDown() ? QColor(0xff, 0x6b, 0x6b) : QColor(0x2a, 0x31, 0x45));
        p.drawRoundedRect(r, 4, 4);
    }

    QPen pen(underMouse() ? QColor(0xff, 0xff, 0xff) : QColor(0x8b, 0x93, 0xa7));
    pen.setWidthF(1.4);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);

    const qreal m = 5.0;
    p.drawLine(QPointF(r.left() + m, r.top() + m),
               QPointF(r.right() - m, r.bottom() - m));
    p.drawLine(QPointF(r.right() - m, r.top() + m),
               QPointF(r.left() + m, r.bottom() - m));
}

} // namespace zen::ui
