#pragma once

#include <QWidget>

class QFileSystemModel;
class QTreeView;
class QLabel;
class QStackedWidget;
class QPushButton;

namespace zen::ui {

class ExplorerPanel : public QWidget {
    Q_OBJECT
public:
    explicit ExplorerPanel(QWidget* parent = nullptr);

    void setRootFolder(const QString& path);
    [[nodiscard]] QString rootFolder() const noexcept { return m_rootFolder; }

signals:
    void fileActivated(const QString& absolutePath);
    void openFolderRequested();

private:
    QLabel*           m_header {nullptr};
    QStackedWidget*   m_stack  {nullptr};
    QWidget*          m_empty  {nullptr};
    QPushButton*      m_openBtn {nullptr};
    QTreeView*        m_tree   {nullptr};
    QFileSystemModel* m_model  {nullptr};
    QString           m_rootFolder;
};

} // namespace zen::ui
