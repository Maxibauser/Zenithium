#include "StatusBar.h"

#include <QHBoxLayout>
#include <QLabel>

namespace zen::ui {

StatusBar::StatusBar(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenStatusBar");
    setFixedHeight(24);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 12, 0);
    layout->setSpacing(12);

    m_message = new QLabel(QStringLiteral("Ready"), this);
    m_message->setObjectName("ZenStatusMessage");
    layout->addWidget(m_message);
    layout->addStretch(1);

    auto* branch = new QLabel(QStringLiteral("⌥  no workspace"), this);
    branch->setObjectName("ZenStatusBranch");
    layout->addWidget(branch);
}

void StatusBar::setMessage(const QString& text) {
    m_message->setText(text);
}

} // namespace zen::ui
