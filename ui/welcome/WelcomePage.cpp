#include "WelcomePage.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "zen/core/Version.h"

namespace zen::ui {

namespace {

QLabel* makeTitle(const QString& text, QWidget* parent, int px, bool bold = false) {
    auto* l = new QLabel(text, parent);
    QFont f = l->font();
    f.setPixelSize(px);
    f.setWeight(bold ? QFont::DemiBold : QFont::Normal);
    l->setFont(f);
    return l;
}

QPushButton* makeAction(const QString& text, QWidget* parent) {
    auto* b = new QPushButton(text, parent);
    b->setObjectName("ZenWelcomeAction");
    b->setCursor(Qt::PointingHandCursor);
    b->setFlat(true);
    return b;
}

} // namespace

WelcomePage::WelcomePage(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenWelcomePage");

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->addStretch(1);

    auto* inner = new QWidget(this);
    inner->setObjectName("ZenWelcomeInner");
    auto* v = new QVBoxLayout(inner);
    v->setContentsMargins(64, 40, 64, 40);
    v->setSpacing(18);

    auto* brand = makeTitle(tr("Zenithium"), inner, 44, true);
    brand->setObjectName("ZenWelcomeBrand");
    v->addWidget(brand);

    const QString versionStr = QString::fromUtf8(
        zen::core::versionString().data(),
        static_cast<qsizetype>(zen::core::versionString().size()));
    auto* sub = makeTitle(tr("A modern cross-platform code IDE  ·  v%1").arg(versionStr),
                          inner, 14);
    sub->setObjectName("ZenWelcomeSub");
    v->addWidget(sub);

    v->addSpacing(24);

    auto* startHeader = makeTitle(tr("Start"), inner, 18, true);
    startHeader->setObjectName("ZenWelcomeSection");
    v->addWidget(startHeader);

    auto* newBtn    = makeAction(tr("New File           Ctrl+N"), inner);
    auto* openBtn   = makeAction(tr("Open File…         Ctrl+O"), inner);
    auto* folderBtn = makeAction(tr("Open Folder…       Ctrl+K Ctrl+O"), inner);
    v->addWidget(newBtn);
    v->addWidget(openBtn);
    v->addWidget(folderBtn);

    v->addSpacing(24);
    auto* tipsHeader = makeTitle(tr("Tips"), inner, 18, true);
    tipsHeader->setObjectName("ZenWelcomeSection");
    v->addWidget(tipsHeader);

    auto* tip1 = makeTitle(tr("• Ctrl+B toggles the file explorer"),      inner, 13);
    auto* tip2 = makeTitle(tr("• Ctrl+, opens settings"),                  inner, 13);
    auto* tip3 = makeTitle(tr("• Change bars in the gutter mark edits since last save"),
                            inner, 13);
    tip1->setObjectName("ZenWelcomeTip");
    tip2->setObjectName("ZenWelcomeTip");
    tip3->setObjectName("ZenWelcomeTip");
    v->addWidget(tip1);
    v->addWidget(tip2);
    v->addWidget(tip3);

    auto* row = new QHBoxLayout();
    row->addStretch(1);
    row->addWidget(inner);
    row->addStretch(1);
    outer->addLayout(row);
    outer->addStretch(2);

    connect(newBtn,    &QPushButton::clicked, this, &WelcomePage::newFileRequested);
    connect(openBtn,   &QPushButton::clicked, this, &WelcomePage::openFileRequested);
    connect(folderBtn, &QPushButton::clicked, this, &WelcomePage::openFolderRequested);
}

} // namespace zen::ui
