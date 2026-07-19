#include "Bootstrap.h"

#include <QApplication>
#include <QFileInfo>
#include <QTimer>

#include "shell/MainWindow.h"
#include "theming/ThemeEngine.h"

namespace zen::app {

void Bootstrap::configureApplication(QApplication& app) {
    QApplication::setOrganizationName(QStringLiteral("Zenithium"));
    QApplication::setOrganizationDomain(QStringLiteral("zenithium.dev"));
    QApplication::setApplicationName(QStringLiteral("Zenithium"));
    QApplication::setApplicationDisplayName(QStringLiteral("Zenithium"));

    zen::ui::ThemeEngine::apply(app, zen::ui::Theme::Dark);
}

int Bootstrap::run(int argc, char** argv) {
    QApplication app(argc, argv);
    configureApplication(app);

    zen::ui::MainWindow window;
    window.show();

    // Any file paths get opened as tabs, any directory
    // becomes the workspace root.
    bool openSettingsAtStart = false;
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QLatin1String("--settings")) {
            openSettingsAtStart = true;
            continue;
        }
        const QFileInfo info(arg);
        if (info.isDir()) {
            window.openWorkspace(info.absoluteFilePath());
        } else if (info.isFile()) {
            window.openFileInTab(info.absoluteFilePath());
        }
    }
    if (openSettingsAtStart) {
        QTimer::singleShot(300, &window, [&window] { window.showSettingsDialog(); });
    }

    return QApplication::exec();
}

} // namespace zen::app
