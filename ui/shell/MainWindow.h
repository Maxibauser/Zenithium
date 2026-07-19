#pragma once

#include <QMainWindow>

#include <memory>
#include <vector>

class QSplitter;
class QStackedWidget;
class QTabBar;

namespace zen::document { class Document; }

namespace zen::ui {

class TitleBar;
class StatusBar;
class EditorView;
class ExplorerPanel;
class SyntaxHighlighter;
class WelcomePage;
class GitPanel;
class TerminalPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void openWorkspace(const QString& folderPath);
    void openFileInTab(const QString& path) { openFile(path); }
    void showSettingsDialog() { openSettings(); }
    void toggleGitPanel(bool on);
    void toggleTerminalPanel(bool on);

private:
    void buildLayout();
    void buildMenus();
    void updateWindowTitle();
    void showWelcomeIfEmpty();
    void applyPreferences();

    void openFileDialog();
    void openFolderDialog();
    void openFile(const QString& path);
    int  addTabForDocument(std::unique_ptr<zen::document::Document> doc,
                           const QString& tabLabel);
    void switchToTab(int index);
    void closeTab(int index);
    void refreshTabLabel(int index);
    void installCloseButton(int index);
    void newUntitledTab();
    void openSettings();

    void saveFile();
    void saveFileAs();

    zen::document::Document* currentDoc() const;

    TitleBar*          m_titleBar   {nullptr};
    StatusBar*         m_statusBar  {nullptr};
    QSplitter*         m_splitter   {nullptr};
    ExplorerPanel*     m_explorer   {nullptr};
    QTabBar*           m_tabs       {nullptr};
    QStackedWidget*    m_stack      {nullptr};
    WelcomePage*       m_welcome    {nullptr};
    EditorView*        m_editor     {nullptr};
    SyntaxHighlighter* m_highlighter{nullptr};
    GitPanel*          m_git        {nullptr};
    TerminalPanel*     m_terminal   {nullptr};
    QSplitter*         m_vsplit     {nullptr};

    std::vector<std::unique_ptr<zen::document::Document>> m_docs;
    int m_currentTab {-1};
};

} // namespace zen::ui
