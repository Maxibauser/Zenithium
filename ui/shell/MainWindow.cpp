#include "MainWindow.h"

#include "CommandPalette.h"
#include "FindBar.h"
#include "StatusBar.h"
#include "TabCloseButton.h"
#include "TitleBar.h"
#include "dialogs/SettingsDialog/Preferences.h"
#include "dialogs/SettingsDialog/SettingsDialog.h"
#include "editor/EditorView.h"
#include "editor/SyntaxHighlighter.h"
#include "panels/Explorer/ExplorerPanel.h"
#include "panels/Git/GitPanel.h"
#include "panels/Terminal/TerminalPanel.h"
#include "theming/ThemeEngine.h"
#include "welcome/WelcomePage.h"

#include <QApplication>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QMenuBar>
#include <QMessageBox>
#include <QSplitter>
#include <QStackedWidget>
#include <QTextDocument>
#include <QTextOption>
#include <QTabBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "zen/core/Version.h"
#include "zen/document/Document.h"
#include "zen/syntax/LanguageRules.h"

namespace zen::ui {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setObjectName("ZenMainWindow");
    setWindowTitle(QStringLiteral("Zenithium"));
    resize(1280, 800);
    buildLayout();
    buildMenus();

    m_highlighter = new SyntaxHighlighter(m_editor->document());
    m_findBar->attachEditor(m_editor);
    m_editor->installEventFilter(this);

    connect(m_explorer, &ExplorerPanel::fileActivated, this, &MainWindow::openFile);
    connect(m_explorer, &ExplorerPanel::openFolderRequested,
            this, &MainWindow::openFolderDialog);

    connect(m_welcome, &WelcomePage::newFileRequested,    this, &MainWindow::newUntitledTab);
    connect(m_welcome, &WelcomePage::openFileRequested,   this, &MainWindow::openFileDialog);
    connect(m_welcome, &WelcomePage::openFolderRequested, this, &MainWindow::openFolderDialog);

    connect(&Preferences::instance(), &Preferences::changed,
            this, &MainWindow::applyPreferences);

    connect(m_titleBar->gitButton(), &QToolButton::toggled,
            this, &MainWindow::toggleGitPanel);
    connect(m_titleBar->terminalButton(), &QToolButton::toggled,
            this, &MainWindow::toggleTerminalPanel);

    connect(m_git, &GitPanel::branchInfoChanged, this,
            [this](const QString& branch, int ahead, int behind) {
                m_statusBar->setBranch(branch, ahead, behind);
            });

    applyPreferences();
    showWelcomeIfEmpty();
}

MainWindow::~MainWindow() = default;

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_editor && event->type() == QEvent::FocusOut) {
        if (Preferences::instance().autoSaveEnabled()) {
            auto* doc = currentDoc();
            if (doc && doc->isModified() && !doc->filePath().isEmpty()) {
                QString err;
                doc->saveToFile(doc->filePath(), &err);
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::openWorkspace(const QString& folderPath) {
    if (folderPath.isEmpty()) return;
    m_explorer->setRootFolder(folderPath);
    m_git->setRepoFolder(folderPath);
    m_terminal->setWorkingDirectory(folderPath);
    m_statusBar->setWorkspaceName(QFileInfo(folderPath).fileName());
    m_statusBar->setMessage(tr("Opened %1").arg(folderPath));
}

void MainWindow::toggleGitPanel(bool on) {
    m_git->setVisible(on);
    if (on) m_git->service()->refreshStatus();
}

void MainWindow::openCommandPalette() {
    if (!m_palette) {
        m_palette = new CommandPalette(this);
        QVector<Command> cmds;
        auto add = [&](const QString& cat, const QString& name,
                       const QString& sc, std::function<void()> fn) {
            cmds.push_back({name, cat, sc, std::move(fn)});
        };

        // File
        add("File", tr("New File"),    "Ctrl+N",       [this] { newUntitledTab(); });
        add("File", tr("Open File…"),  "Ctrl+O",       [this] { openFileDialog(); });
        add("File", tr("Open Folder…"),"Ctrl+K Ctrl+O",[this] { openFolderDialog(); });
        add("File", tr("Save"),        "Ctrl+S",       [this] { saveFile(); });
        add("File", tr("Save As…"),    "Ctrl+Shift+S", [this] { saveFileAs(); });
        add("File", tr("Close Tab"),   "Ctrl+W",       [this] {
            if (m_currentTab >= 0) closeTab(m_currentTab);
        });

        // Edit
        add("Edit", tr("Undo"),  "Ctrl+Z", [this] { m_editor->undo(); });
        add("Edit", tr("Redo"),  "Ctrl+Y", [this] { m_editor->redo(); });
        add("Edit", tr("Find…"), "Ctrl+F", [this] { m_findBar->showAndFocus(); });

        // View / Panels
        add("View", tr("Toggle Explorer"),
            "Ctrl+B",       [this] { m_explorer->setVisible(!m_explorer->isVisible()); });
        add("View", tr("Toggle Source Control"),
            "Ctrl+Shift+G", [this] { m_titleBar->gitButton()->toggle(); });
        add("View", tr("Toggle Terminal"),
            "Ctrl+`",       [this] { m_titleBar->terminalButton()->toggle(); });
        add("View", tr("Welcome Page"), QString(), [this] {
            m_tabs->setCurrentIndex(-1);
            m_stack->setCurrentWidget(m_welcome);
            m_currentTab = -1;
            updateWindowTitle();
        });

        // Settings
        add("Settings", tr("Preferences…"), "Ctrl+,",
            [this] { openSettings(); });
        add("Settings", tr("Switch to Dark Theme"), QString(),
            [] { Preferences::instance().setTheme("dark"); });
        add("Settings", tr("Switch to Light Theme"), QString(),
            [] { Preferences::instance().setTheme("light"); });
        add("Settings", tr("Toggle Syntax Highlighting"), QString(), [] {
            auto& p = Preferences::instance();
            p.setSyntaxHighlightingEnabled(!p.syntaxHighlightingEnabled());
        });
        add("Settings", tr("Toggle Line Numbers"), QString(), [] {
            auto& p = Preferences::instance();
            p.setLineNumbersEnabled(!p.lineNumbersEnabled());
        });
        add("Settings", tr("Toggle Change Bars"), QString(), [] {
            auto& p = Preferences::instance();
            p.setChangeBarsEnabled(!p.changeBarsEnabled());
        });
        add("Settings", tr("Toggle Word Wrap"), QString(), [] {
            auto& p = Preferences::instance();
            p.setWordWrap(!p.wordWrap());
        });
        add("Settings", tr("Toggle Show Whitespace"), QString(), [] {
            auto& p = Preferences::instance();
            p.setShowWhitespace(!p.showWhitespace());
        });
        add("Settings", tr("Toggle Auto-Save"), QString(), [] {
            auto& p = Preferences::instance();
            p.setAutoSaveEnabled(!p.autoSaveEnabled());
        });

        // Git
        auto* svc = m_git->service();
        add("Git", tr("Refresh Status"), QString(), [svc] { svc->refreshStatus(); });
        add("Git", tr("Fetch"),          QString(), [svc] { svc->fetch(); });
        add("Git", tr("Pull"),           QString(), [svc] { svc->pull(); });
        add("Git", tr("Push"),           QString(), [svc] { svc->push(); });
        add("Git", tr("Stage All"),      QString(), [svc] { svc->stageAll(); });
        add("Git", tr("Initialize Repository"), QString(), [svc] { svc->initRepo(); });

        m_palette->setCommands(std::move(cmds));
    }
    m_palette->showPalette();
}

void MainWindow::toggleTerminalPanel(bool on) {
    m_terminal->setVisible(on);
    if (on) {
        // Give the terminal a reasonable initial size the first time it opens.
        auto sizes = m_vsplit->sizes();
        if (sizes.size() == 2 && sizes[1] < 60) {
            const int total = sizes[0] + sizes[1];
            const int termH = qMax(220, total / 4);
            m_vsplit->setSizes({total - termH, termH});
        }
    }
}

void MainWindow::buildLayout() {
    auto* central = new QWidget(this);
    central->setObjectName("ZenCentral");
    auto* layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_titleBar = new TitleBar(central);
    layout->addWidget(m_titleBar);

    m_splitter = new QSplitter(Qt::Horizontal, central);
    m_splitter->setObjectName("ZenSplitter");
    m_splitter->setHandleWidth(1);
    m_splitter->setChildrenCollapsible(false);

    m_explorer = new ExplorerPanel(m_splitter);

    auto* right = new QWidget(m_splitter);
    right->setObjectName("ZenEditorHost");
    auto* rightLayout = new QVBoxLayout(right);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(0);

    m_tabs = new QTabBar(right);
    m_tabs->setObjectName("ZenTabBar");
    m_tabs->setTabsClosable(false); // we install a custom button per tab
    m_tabs->setMovable(true);
    m_tabs->setExpanding(false);
    m_tabs->setDrawBase(false);
    m_tabs->setUsesScrollButtons(true);
    m_tabs->setElideMode(Qt::ElideRight);
    m_tabs->setMinimumHeight(34);
    m_tabs->setAutoHide(false);
    connect(m_tabs, &QTabBar::currentChanged, this, &MainWindow::switchToTab);
    rightLayout->addWidget(m_tabs);

    m_findBar = new FindBar(right);
    rightLayout->addWidget(m_findBar);

    m_stack = new QStackedWidget(right);
    m_welcome = new WelcomePage(m_stack);
    m_editor  = new EditorView(m_stack);
    m_stack->addWidget(m_welcome);
    m_stack->addWidget(m_editor);
    rightLayout->addWidget(m_stack, /*stretch*/ 1);

    m_git = new GitPanel(m_splitter);
    m_git->hide();

    m_splitter->addWidget(m_explorer);
    m_splitter->addWidget(right);
    m_splitter->addWidget(m_git);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setStretchFactor(2, 0);
    m_splitter->setSizes({260, 720, 300});

    m_vsplit = new QSplitter(Qt::Vertical, central);
    m_vsplit->setObjectName("ZenVSplitter");
    m_vsplit->setHandleWidth(1);
    m_vsplit->setChildrenCollapsible(false);
    m_vsplit->addWidget(m_splitter);

    m_terminal = new TerminalPanel(m_vsplit);
    m_terminal->hide();
    m_vsplit->addWidget(m_terminal);
    m_vsplit->setStretchFactor(0, 1);
    m_vsplit->setStretchFactor(1, 0);
    m_vsplit->setSizes({600, 0});

    layout->addWidget(m_vsplit, /*stretch*/ 1);

    m_statusBar = new StatusBar(central);
    layout->addWidget(m_statusBar);

    setCentralWidget(central);
}

void MainWindow::buildMenus() {
    auto* file = menuBar()->addMenu(tr("&File"));
    file->addAction(tr("New File"),   QKeySequence::New, this, &MainWindow::newUntitledTab);
    file->addAction(tr("Open File…"), QKeySequence::Open, this, &MainWindow::openFileDialog);
    file->addAction(tr("Open Folder…"),
                    QKeySequence(QStringLiteral("Ctrl+K, Ctrl+O")),
                    this, &MainWindow::openFolderDialog);
    file->addSeparator();
    file->addAction(tr("Save"),       QKeySequence::Save, this, &MainWindow::saveFile);
    file->addAction(tr("Save As…"),   QKeySequence(QStringLiteral("Ctrl+Shift+S")),
                    this, &MainWindow::saveFileAs);
    file->addSeparator();
    file->addAction(tr("Close Tab"),  QKeySequence(QStringLiteral("Ctrl+W")),
                    [this] { if (m_currentTab >= 0) closeTab(m_currentTab); });
    file->addAction(tr("Exit"), QKeySequence(QStringLiteral("Ctrl+Q")), this, &QWidget::close);

    auto* edit = menuBar()->addMenu(tr("&Edit"));
    edit->addAction(tr("Undo"), QKeySequence::Undo, m_editor, &QPlainTextEdit::undo);
    edit->addAction(tr("Redo"), QKeySequence::Redo, m_editor, &QPlainTextEdit::redo);
    edit->addSeparator();
    edit->addAction(tr("Cut"),   QKeySequence::Cut,   m_editor, &QPlainTextEdit::cut);
    edit->addAction(tr("Copy"),  QKeySequence::Copy,  m_editor, &QPlainTextEdit::copy);
    edit->addAction(tr("Paste"), QKeySequence::Paste, m_editor, &QPlainTextEdit::paste);
    edit->addSeparator();
    edit->addAction(tr("Find…"), QKeySequence::Find, this,
                    [this] { m_findBar->showAndFocus(); });

    auto* view = menuBar()->addMenu(tr("&View"));
    view->addAction(tr("Toggle Explorer"),
                    QKeySequence(QStringLiteral("Ctrl+B")),
                    [this] { m_explorer->setVisible(!m_explorer->isVisible()); });
    view->addAction(tr("Command Palette"),
                    QKeySequence(QStringLiteral("Ctrl+Shift+P")),
                    this, &MainWindow::openCommandPalette);
    view->addAction(tr("Toggle Source Control"),
                    QKeySequence(QStringLiteral("Ctrl+Shift+G")),
                    [this] { m_titleBar->gitButton()->toggle(); });
    view->addAction(tr("Toggle Terminal"),
                    QKeySequence(QStringLiteral("Ctrl+`")),
                    [this] { m_titleBar->terminalButton()->toggle(); });
    view->addAction(tr("Welcome Page"), [this] {
        m_tabs->setCurrentIndex(-1);
        m_stack->setCurrentWidget(m_welcome);
        m_currentTab = -1;
        updateWindowTitle();
    });

    auto* settings = menuBar()->addMenu(tr("&Settings"));
    settings->addAction(tr("Preferences…"),
                        QKeySequence(QStringLiteral("Ctrl+,")),
                        this, &MainWindow::openSettings);
    settings->addSeparator();
    auto* syntaxToggle = settings->addAction(tr("Syntax Highlighting"));
    syntaxToggle->setCheckable(true);
    syntaxToggle->setChecked(Preferences::instance().syntaxHighlightingEnabled());
    connect(syntaxToggle, &QAction::toggled,
            &Preferences::instance(), &Preferences::setSyntaxHighlightingEnabled);
    connect(&Preferences::instance(), &Preferences::changed, syntaxToggle,
            [syntaxToggle] { QSignalBlocker b(syntaxToggle);
                             syntaxToggle->setChecked(
                                 Preferences::instance().syntaxHighlightingEnabled()); });

    auto* changeToggle = settings->addAction(tr("Change Bars"));
    changeToggle->setCheckable(true);
    changeToggle->setChecked(Preferences::instance().changeBarsEnabled());
    connect(changeToggle, &QAction::toggled,
            &Preferences::instance(), &Preferences::setChangeBarsEnabled);
    connect(&Preferences::instance(), &Preferences::changed, changeToggle,
            [changeToggle] { QSignalBlocker b(changeToggle);
                             changeToggle->setChecked(
                                 Preferences::instance().changeBarsEnabled()); });

    auto* linesToggle = settings->addAction(tr("Line Numbers"));
    linesToggle->setCheckable(true);
    linesToggle->setChecked(Preferences::instance().lineNumbersEnabled());
    connect(linesToggle, &QAction::toggled,
            &Preferences::instance(), &Preferences::setLineNumbersEnabled);
    connect(&Preferences::instance(), &Preferences::changed, linesToggle,
            [linesToggle] { QSignalBlocker b(linesToggle);
                            linesToggle->setChecked(
                                Preferences::instance().lineNumbersEnabled()); });

    auto* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(tr("About Zenithium"), [this] {
        QMessageBox::about(this, tr("About Zenithium"),
            tr("<h2>Zenithium</h2><p>Version %1</p>"
               "<p>A modern cross-platform code IDE.</p>")
                .arg(QString::fromUtf8(zen::core::versionString().data(),
                                       static_cast<qsizetype>(zen::core::versionString().size()))));
    });
}

void MainWindow::openSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::applyPreferences() {
    const auto& p = Preferences::instance();

    // Theme + accent
    zen::ui::ThemeEngine::apply(*qApp,
        p.theme() == QLatin1String("light") ? zen::ui::Theme::Light
                                            : zen::ui::Theme::Dark);

    // Editor font (family + size) and tab width
    QFont f = m_editor->font();
    if (!p.editorFontFamily().isEmpty()) f.setFamily(p.editorFontFamily());
    f.setPointSize(p.editorFontSize());
    m_editor->setFont(f);
    m_editor->setTabStopDistance(
        p.tabWidth() * QFontMetrics(f).horizontalAdvance(QLatin1Char(' ')));

    // Word wrap
    m_editor->setLineWrapMode(p.wordWrap()
        ? QPlainTextEdit::WidgetWidth
        : QPlainTextEdit::NoWrap);

    // Show whitespace
    {
        QTextOption opt = m_editor->document()->defaultTextOption();
        auto flags = opt.flags();
        flags.setFlag(QTextOption::ShowTabsAndSpaces, p.showWhitespace());
        opt.setFlags(flags);
        m_editor->document()->setDefaultTextOption(opt);
    }

    // Syntax highlighting on/off
    if (p.syntaxHighlightingEnabled()) {
        if (auto* doc = currentDoc()) {
            m_highlighter->setLanguageForPath(doc->filePath());
        }
    } else {
        m_highlighter->setLanguage(zen::syntax::rulesByName(QStringLiteral("plain")));
    }

    // Line numbers + change bars — expose via EditorView setters
    m_editor->setLineNumbersVisible(p.lineNumbersEnabled());
    m_editor->setChangeBarsVisible(p.changeBarsEnabled());

    // Modified indicator (refresh all tab labels + title)
    for (int i = 0; i < static_cast<int>(m_docs.size()); ++i) {
        refreshTabLabel(i);
    }
    updateWindowTitle();
}

int MainWindow::addTabForDocument(std::unique_ptr<zen::document::Document> doc,
                                  const QString& tabLabel) {
    auto* raw = doc.get();
    m_docs.push_back(std::move(doc));
    const int index = m_tabs->addTab(tabLabel);
    m_tabs->setTabToolTip(index, raw->filePath().isEmpty() ? tabLabel : raw->filePath());
    installCloseButton(index);

    connect(raw, &zen::document::Document::modifiedChanged, this,
            [this, raw] {
                for (int i = 0; i < static_cast<int>(m_docs.size()); ++i) {
                    if (m_docs[i].get() == raw) { refreshTabLabel(i); break; }
                }
            });
    connect(raw, &zen::document::Document::filePathChanged, this,
            [this, raw](const QString& p) {
                for (int i = 0; i < static_cast<int>(m_docs.size()); ++i) {
                    if (m_docs[i].get() == raw) {
                        m_tabs->setTabToolTip(i, p);
                        refreshTabLabel(i);
                        break;
                    }
                }
                if (raw == currentDoc() &&
                    Preferences::instance().syntaxHighlightingEnabled()) {
                    m_highlighter->setLanguageForPath(p);
                    updateWindowTitle();
                }
            });

    m_stack->setCurrentWidget(m_editor);
    m_tabs->setCurrentIndex(index);
    return index;
}

void MainWindow::installCloseButton(int index) {
    auto* btn = new TabCloseButton();
    connect(btn, &QToolButton::clicked, this, [this, btn] {
        // Resolve the tab index at click-time because tabs may have moved.
        for (int i = 0; i < m_tabs->count(); ++i) {
            if (m_tabs->tabButton(i, QTabBar::RightSide) == btn) {
                closeTab(i);
                return;
            }
        }
    });
    m_tabs->setTabButton(index, QTabBar::RightSide, btn);
}

void MainWindow::switchToTab(int index) {
    if (index < 0 || index >= static_cast<int>(m_docs.size())) {
        m_currentTab = -1;
        m_editor->attachDocument(nullptr);
        return;
    }
    m_currentTab = index;
    m_stack->setCurrentWidget(m_editor);
    auto* doc = m_docs[static_cast<size_t>(index)].get();
    m_editor->attachDocument(doc);
    if (Preferences::instance().syntaxHighlightingEnabled()) {
        m_highlighter->setLanguageForPath(doc->filePath());
    } else {
        m_highlighter->setLanguage(zen::syntax::rulesByName(QStringLiteral("plain")));
    }
    updateWindowTitle();
}

void MainWindow::closeTab(int index) {
    if (index < 0 || index >= static_cast<int>(m_docs.size())) return;

    auto* doc = m_docs[static_cast<size_t>(index)].get();
    if (doc->isModified()) {
        const auto btn = QMessageBox::question(
            this, tr("Unsaved changes"),
            tr("%1 has unsaved changes. Close anyway?")
                .arg(doc->filePath().isEmpty()
                         ? tr("Untitled")
                         : QFileInfo(doc->filePath()).fileName()),
            QMessageBox::Yes | QMessageBox::No);
        if (btn != QMessageBox::Yes) return;
    }

    m_tabs->removeTab(index);
    m_docs.erase(m_docs.begin() + index);
    showWelcomeIfEmpty();
}

void MainWindow::showWelcomeIfEmpty() {
    if (m_docs.empty()) {
        m_editor->attachDocument(nullptr);
        m_stack->setCurrentWidget(m_welcome);
        m_currentTab = -1;
        setWindowTitle(QStringLiteral("Zenithium"));
    }
}

void MainWindow::refreshTabLabel(int index) {
    if (index < 0 || index >= static_cast<int>(m_docs.size())) return;
    auto* doc = m_docs[static_cast<size_t>(index)].get();
    QString label = doc->filePath().isEmpty()
                      ? tr("Untitled")
                      : QFileInfo(doc->filePath()).fileName();
    if (doc->isModified() && Preferences::instance().modifiedIndicatorEnabled()) {
        label.prepend(QStringLiteral("● "));
    }
    m_tabs->setTabText(index, label);
    if (index == m_currentTab) updateWindowTitle();
}

void MainWindow::newUntitledTab() {
    auto doc = std::make_unique<zen::document::Document>();
    addTabForDocument(std::move(doc), tr("Untitled"));
    m_editor->setFocus();
}

zen::document::Document* MainWindow::currentDoc() const {
    if (m_currentTab < 0 || m_currentTab >= static_cast<int>(m_docs.size())) {
        return nullptr;
    }
    return m_docs[static_cast<size_t>(m_currentTab)].get();
}

void MainWindow::updateWindowTitle() {
    auto* doc = currentDoc();
    if (!doc) {
        setWindowTitle(QStringLiteral("Zenithium"));
        return;
    }
    QString name = doc->filePath().isEmpty()
                       ? tr("Untitled")
                       : QFileInfo(doc->filePath()).fileName();
    if (doc->isModified() && Preferences::instance().modifiedIndicatorEnabled()) {
        name.prepend(QStringLiteral("● "));
    }
    setWindowTitle(QStringLiteral("%1 — Zenithium").arg(name));
}

void MainWindow::openFileDialog() {
    const QString start = m_explorer->rootFolder();
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open File"), start,
        tr("All Files (*);;C/C++ (*.h *.hpp *.c *.cpp *.cc);;Python (*.py);;JSON (*.json);;Text (*.txt *.md)"));
    if (path.isEmpty()) return;
    openFile(path);
}

void MainWindow::openFolderDialog() {
    const QString dir = QFileDialog::getExistingDirectory(this, tr("Open Folder"));
    if (dir.isEmpty()) return;
    m_explorer->setRootFolder(dir);
    m_git->setRepoFolder(dir);
    m_terminal->setWorkingDirectory(dir);
    m_statusBar->setWorkspaceName(QFileInfo(dir).fileName());
    m_statusBar->setMessage(tr("Opened %1").arg(dir));
}

void MainWindow::openFile(const QString& path) {
    const QString abs = QFileInfo(path).absoluteFilePath();
    for (int i = 0; i < static_cast<int>(m_docs.size()); ++i) {
        if (m_docs[i]->filePath() == abs) {
            m_tabs->setCurrentIndex(i);
            return;
        }
    }

    auto doc = std::make_unique<zen::document::Document>();
    QString error;
    if (!doc->loadFromFile(path, &error)) {
        QMessageBox::warning(this, tr("Open failed"), error);
        return;
    }
    const QString label = QFileInfo(path).fileName();
    addTabForDocument(std::move(doc), label);
    m_statusBar->setMessage(tr("Opened %1").arg(label));
}

void MainWindow::saveFile() {
    auto* doc = currentDoc();
    if (!doc) return;
    if (doc->filePath().isEmpty()) {
        saveFileAs();
        return;
    }
    QString error;
    if (!doc->saveToFile(doc->filePath(), &error)) {
        QMessageBox::warning(this, tr("Save failed"), error);
        return;
    }
    m_statusBar->setMessage(tr("Saved"));
}

void MainWindow::saveFileAs() {
    auto* doc = currentDoc();
    if (!doc) return;
    const QString start = doc->filePath().isEmpty()
                              ? m_explorer->rootFolder()
                              : doc->filePath();
    const QString path = QFileDialog::getSaveFileName(
        this, tr("Save As"), start, tr("All Files (*)"));
    if (path.isEmpty()) return;

    QString error;
    if (!doc->saveToFile(path, &error)) {
        QMessageBox::warning(this, tr("Save failed"), error);
        return;
    }
    m_statusBar->setMessage(tr("Saved as %1").arg(QFileInfo(path).fileName()));
}

} // namespace zen::ui
