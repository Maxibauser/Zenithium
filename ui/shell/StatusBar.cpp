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

    m_workspace = new QLabel(this);
    m_workspace->setObjectName("ZenStatusChip");
    m_workspace->hide();
    layout->addWidget(m_workspace);

    m_branch = new QLabel(this);
    m_branch->setObjectName("ZenStatusChip");
    m_branch->hide();
    layout->addWidget(m_branch);
}

void StatusBar::setMessage(const QString& text) {
    m_message->setText(text);
}

void StatusBar::setWorkspaceName(const QString& name) {
    if (name.isEmpty()) { m_workspace->hide(); return; }
    m_workspace->setText(QStringLiteral("🗀  %1").arg(name));
    m_workspace->show();
}

void StatusBar::setBranch(const QString& branch, int ahead, int behind) {
    if (branch.isEmpty()) { m_branch->hide(); return; }
    QString text = QStringLiteral("⎇  %1").arg(branch);
    if (ahead)  text += QStringLiteral("  ↑%1").arg(ahead);
    if (behind) text += QStringLiteral("  ↓%1").arg(behind);
    m_branch->setText(text);
    m_branch->show();
}

} // namespace zen::ui
