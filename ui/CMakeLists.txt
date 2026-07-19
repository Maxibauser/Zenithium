add_library(zen_ui STATIC
    shell/MainWindow.h
    shell/MainWindow.cpp
    shell/TitleBar.h
    shell/TitleBar.cpp
    shell/FindBar.h
    shell/FindBar.cpp
    shell/StatusBar.h
    shell/StatusBar.cpp
    shell/TabCloseButton.h
    shell/TabCloseButton.cpp
    theming/ThemeEngine.h
    theming/ThemeEngine.cpp
    editor/EditorView.h
    editor/EditorView.cpp
    editor/Gutter.h
    editor/Gutter.cpp
    editor/SyntaxHighlighter.h
    editor/SyntaxHighlighter.cpp
    panels/Explorer/ExplorerPanel.h
    panels/Explorer/ExplorerPanel.cpp
    panels/Git/GitPanel.h
    panels/Git/GitPanel.cpp
    panels/Terminal/TerminalPanel.h
    panels/Terminal/TerminalPanel.cpp
    welcome/WelcomePage.h
    welcome/WelcomePage.cpp
    dialogs/SettingsDialog/SettingsDialog.h
    dialogs/SettingsDialog/SettingsDialog.cpp
    dialogs/SettingsDialog/Preferences.h
    dialogs/SettingsDialog/Preferences.cpp
    resources/zenithium.qrc
)
add_library(zen::ui ALIAS zen_ui)

target_include_directories(zen_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(zen_ui
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        zen::core
        zen::document
        zen::syntax
        zen::git
)

zen_target_defaults(zen_ui)
zen_enable_sanitizers(zen_ui)
