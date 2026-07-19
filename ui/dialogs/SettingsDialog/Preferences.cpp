#include "Preferences.h"

#include <QSettings>

namespace zen::ui {

Preferences& Preferences::instance() {
    static Preferences p;
    return p;
}

Preferences::Preferences() {
    QSettings s;
    m_syntaxOn     = s.value("editor/syntax", true).toBool();
    m_changeBars   = s.value("editor/changeBars", true).toBool();
    m_lineNumbers  = s.value("editor/lineNumbers", true).toBool();
    m_modIndicator = s.value("editor/modifiedIndicator", true).toBool();
    m_fontSize     = s.value("editor/fontSize", 11).toInt();
    m_fontFamily   = s.value("editor/fontFamily", "Cascadia Mono").toString();
    m_tabWidth     = s.value("editor/tabWidth", 4).toInt();
    m_wordWrap     = s.value("editor/wordWrap", false).toBool();
    m_showWhitespace = s.value("editor/showWhitespace", false).toBool();
    m_autoSave     = s.value("editor/autoSave", false).toBool();
    m_theme        = s.value("ui/theme", "dark").toString();
    m_accent       = QColor(s.value("ui/accent", "#6ea8ff").toString());
}

void Preferences::save() const {
    QSettings s;
    s.setValue("editor/syntax",             m_syntaxOn);
    s.setValue("editor/changeBars",         m_changeBars);
    s.setValue("editor/lineNumbers",        m_lineNumbers);
    s.setValue("editor/modifiedIndicator",  m_modIndicator);
    s.setValue("editor/fontSize",           m_fontSize);
    s.setValue("editor/fontFamily",         m_fontFamily);
    s.setValue("editor/tabWidth",           m_tabWidth);
    s.setValue("editor/wordWrap",           m_wordWrap);
    s.setValue("editor/showWhitespace",     m_showWhitespace);
    s.setValue("editor/autoSave",           m_autoSave);
    s.setValue("ui/theme",                  m_theme);
    s.setValue("ui/accent",                 m_accent.name());
}

#define ZEN_SETTER(field, param)                          \
    if (field == param) return;                           \
    field = param;                                        \
    save();                                               \
    emit changed();

void Preferences::setSyntaxHighlightingEnabled(bool on)     { ZEN_SETTER(m_syntaxOn,    on) }
void Preferences::setChangeBarsEnabled(bool on)             { ZEN_SETTER(m_changeBars,  on) }
void Preferences::setLineNumbersEnabled(bool on)            { ZEN_SETTER(m_lineNumbers, on) }
void Preferences::setModifiedIndicatorEnabled(bool on)      { ZEN_SETTER(m_modIndicator,on) }
void Preferences::setEditorFontSize(int pt)                 { ZEN_SETTER(m_fontSize,    pt) }
void Preferences::setEditorFontFamily(const QString& family){ ZEN_SETTER(m_fontFamily,  family) }
void Preferences::setTabWidth(int spaces)                   { ZEN_SETTER(m_tabWidth,    spaces) }
void Preferences::setWordWrap(bool on)                      { ZEN_SETTER(m_wordWrap,    on) }
void Preferences::setShowWhitespace(bool on)                { ZEN_SETTER(m_showWhitespace, on) }
void Preferences::setAutoSaveEnabled(bool on)               { ZEN_SETTER(m_autoSave,    on) }
void Preferences::setTheme(const QString& name)             { ZEN_SETTER(m_theme,       name) }
void Preferences::setAccentColor(const QColor& c)           { ZEN_SETTER(m_accent,      c) }

#undef ZEN_SETTER

} // namespace zen::ui
