#include "ThemeEngine.h"

#include "dialogs/SettingsDialog/Preferences.h"

#include <QApplication>
#include <QFile>
#include <QFontDatabase>
#include <QPalette>
#include <QTextStream>

namespace zen::ui {

namespace {

QPalette makeDarkPalette(const QColor& accent) {
    QPalette p;
    const QColor bg      (0x0f, 0x12, 0x18);
    const QColor surface (0x16, 0x1a, 0x22);
    const QColor elevated(0x1c, 0x21, 0x30);
    const QColor text    (0xe6, 0xe9, 0xf2);
    const QColor subtle  (0x8b, 0x93, 0xa7);

    p.setColor(QPalette::Window,          bg);
    p.setColor(QPalette::WindowText,      text);
    p.setColor(QPalette::Base,            surface);
    p.setColor(QPalette::AlternateBase,   elevated);
    p.setColor(QPalette::ToolTipBase,     elevated);
    p.setColor(QPalette::ToolTipText,     text);
    p.setColor(QPalette::Text,            text);
    p.setColor(QPalette::PlaceholderText, subtle);
    p.setColor(QPalette::Button,          elevated);
    p.setColor(QPalette::ButtonText,      text);
    p.setColor(QPalette::BrightText,      Qt::white);
    p.setColor(QPalette::Highlight,       accent);
    p.setColor(QPalette::HighlightedText, Qt::black);
    p.setColor(QPalette::Link,            accent);
    return p;
}

QPalette makeLightPalette(const QColor& accent) {
    QPalette p;
    const QColor bg      (0xf7, 0xf8, 0xfb);
    const QColor surface (0xff, 0xff, 0xff);
    const QColor elevated(0xee, 0xf0, 0xf5);
    const QColor text    (0x1a, 0x1f, 0x2c);
    const QColor subtle  (0x66, 0x6f, 0x86);

    p.setColor(QPalette::Window,          bg);
    p.setColor(QPalette::WindowText,      text);
    p.setColor(QPalette::Base,            surface);
    p.setColor(QPalette::AlternateBase,   elevated);
    p.setColor(QPalette::ToolTipBase,     surface);
    p.setColor(QPalette::ToolTipText,     text);
    p.setColor(QPalette::Text,            text);
    p.setColor(QPalette::PlaceholderText, subtle);
    p.setColor(QPalette::Button,          elevated);
    p.setColor(QPalette::ButtonText,      text);
    p.setColor(QPalette::BrightText,      Qt::black);
    p.setColor(QPalette::Highlight,       accent);
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::Link,            accent);
    return p;
}

} // namespace

QString ThemeEngine::loadQss(Theme theme) {
    const QString path = (theme == Theme::Dark)
        ? QStringLiteral(":/zenithium/qss/dark.qss")
        : QStringLiteral(":/zenithium/qss/light.qss");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QTextStream in(&file);
    return in.readAll();
}

QString ThemeEngine::substituteTokens(QString qss) {
    const QColor accent = Preferences::instance().accentColor();
    // Swap the built-in blue accent for whatever the user picked.
    qss.replace(QLatin1String("#6ea8ff"), accent.name());
    // Also derive hover / pressed variants (lighter / darker).
    qss.replace(QLatin1String("#86b7ff"), accent.lighter(115).name());
    qss.replace(QLatin1String("#5a95ee"), accent.darker(115).name());
    return qss;
}

void ThemeEngine::apply(QApplication& app, Theme theme) {
    const QColor accent = Preferences::instance().accentColor();
    app.setStyle(QStringLiteral("Fusion"));
    app.setPalette(theme == Theme::Dark ? makeDarkPalette(accent)
                                        : makeLightPalette(accent));
    const QString qss = substituteTokens(loadQss(theme));
    app.setStyleSheet(qss);
}

} // namespace zen::ui
