#include "TitleBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace zen::ui {

TitleBar::TitleBar(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenTitleBar");
    setFixedHeight(36);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(8);

    auto* brand = new QLabel(QStringLiteral("Zenithium"), this);
    brand->setObjectName("ZenBrand");
    layout->addWidget(brand);
    layout->addStretch(1);

    m_gitBtn = new QToolButton(this);
    m_gitBtn->setObjectName("ZenTitleGitBtn");
    // Simple branch glyph — no external icon file needed.
    m_gitBtn->setText(QStringLiteral("⎇  Git"));
    m_gitBtn->setToolTip(tr("Toggle Source Control panel"));
    m_gitBtn->setCheckable(true);
    m_gitBtn->setAutoRaise(true);
    m_gitBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_gitBtn);

    m_termBtn = new QToolButton(this);
    m_termBtn->setObjectName("ZenTitleTermBtn");
    m_termBtn->setText(QStringLiteral("▷_ Terminal"));
    m_termBtn->setToolTip(tr("Toggle Terminal panel"));
    m_termBtn->setCheckable(true);
    m_termBtn->setAutoRaise(true);
    m_termBtn->setCursor(Qt::PointingHandCursor);
    layout->addWidget(m_termBtn);
}

} // namespace zen::ui
