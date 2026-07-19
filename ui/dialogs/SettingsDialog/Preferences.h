#pragma once

#include <QColor>
#include <QObject>
#include <QString>

namespace zen::ui {

// Process-wide user preferences. Loaded from QSettings on construction,
// written back on any setter. Widgets listen to `changed()` and re-apply.
class Preferences : public QObject {
    Q_OBJECT
public:
    static Preferences& instance();

    // Editor
    bool syntaxHighlightingEnabled() const noexcept { return m_syntaxOn; }
    void setSyntaxHighlightingEnabled(bool on);

    bool changeBarsEnabled() const noexcept { return m_changeBars; }
    void setChangeBarsEnabled(bool on);

    bool lineNumbersEnabled() const noexcept { return m_lineNumbers; }
    void setLineNumbersEnabled(bool on);

    bool modifiedIndicatorEnabled() const noexcept { return m_modIndicator; }
    void setModifiedIndicatorEnabled(bool on);

    int editorFontSize() const noexcept { return m_fontSize; }
    void setEditorFontSize(int pt);

    // Theme
    QString theme() const noexcept  { return m_theme; }    // "dark" / "light"
    void setTheme(const QString& name);

    QColor accentColor() const noexcept { return m_accent; }
    void setAccentColor(const QColor& c);

signals:
    void changed();

private:
    Preferences();
    void save() const;

    bool    m_syntaxOn    {true};
    bool    m_changeBars  {true};
    bool    m_lineNumbers {true};
    bool    m_modIndicator{true};
    int     m_fontSize    {11};
    QString m_theme       {"dark"};
    QColor  m_accent      {0x6e, 0xa8, 0xff};
};

} // namespace zen::ui
