#include "ExplorerPanel.h"

#include <QDir>
#include <QFileInfo>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QTreeView>
#include <QVBoxLayout>

namespace zen::ui {

ExplorerPanel::ExplorerPanel(QWidget* parent) : QWidget(parent) {
    setObjectName("ZenExplorer");

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_header = new QLabel(tr("EXPLORER"), this);
    m_header->setObjectName("ZenExplorerHeader");
    m_header->setContentsMargins(14, 10, 14, 10);
    layout->addWidget(m_header);

    m_stack = new QStackedWidget(this);
    layout->addWidget(m_stack, /*stretch*/ 1);

    // --- Empty state ---
    m_empty = new QWidget(m_stack);
    m_empty->setObjectName("ZenExplorerEmpty");
    auto* emptyLayout = new QVBoxLayout(m_empty);
    emptyLayout->setContentsMargins(18, 8, 18, 18);
    emptyLayout->setSpacing(12);

    auto* hint = new QLabel(
        tr("You have not opened a folder yet."), m_empty);
    hint->setObjectName("ZenExplorerHint");
    hint->setWordWrap(true);
    emptyLayout->addWidget(hint);

    m_openBtn = new QPushButton(tr("Open Folder"), m_empty);
    m_openBtn->setObjectName("ZenExplorerOpenBtn");
    m_openBtn->setCursor(Qt::PointingHandCursor);
    connect(m_openBtn, &QPushButton::clicked,
            this, &ExplorerPanel::openFolderRequested);
    emptyLayout->addWidget(m_openBtn);
    emptyLayout->addStretch(1);

    m_stack->addWidget(m_empty);

    // --- Tree view ---
    m_model = new QFileSystemModel(this);
    m_model->setReadOnly(true);
    m_model->setFilter(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);

    m_tree = new QTreeView(m_stack);
    m_tree->setObjectName("ZenExplorerTree");
    m_tree->setModel(m_model);
    m_tree->setHeaderHidden(true);
    m_tree->setAnimated(true);
    m_tree->setIndentation(14);
    m_tree->setFrameShape(QFrame::NoFrame);
    m_tree->setUniformRowHeights(true);
    m_tree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    for (int i = 1; i < m_model->columnCount(); ++i) {
        m_tree->hideColumn(i);
    }
    m_stack->addWidget(m_tree);

    connect(m_tree, &QTreeView::activated, this, [this](const QModelIndex& idx) {
        const QFileInfo info(m_model->filePath(idx));
        if (info.isFile()) {
            emit fileActivated(info.absoluteFilePath());
        }
    });

    m_stack->setCurrentWidget(m_empty);
}

void ExplorerPanel::setRootFolder(const QString& path) {
    m_rootFolder = path;
    const QModelIndex root = m_model->setRootPath(path);
    m_tree->setRootIndex(root);
    m_header->setText(QFileInfo(path).fileName().toUpper());
    m_stack->setCurrentWidget(m_tree);
}

} // namespace zen::ui
