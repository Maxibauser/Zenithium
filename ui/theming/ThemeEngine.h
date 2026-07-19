#pragma once

#include <QString>

class QApplication;

namespace zen::ui {

enum class Theme {
    Dark,
    Light,
};

class ThemeEngine {
public:
    static void apply(QApplication& app, Theme theme = Theme::Dark);

private:
    static QString loadQss(Theme theme);
    static QString substituteTokens(QString qss);
};

} // namespace zen::ui
