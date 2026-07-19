#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QSpinBox;

namespace zen::ui {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    void loadFromPrefs();
    void applyToPrefs();

    QCheckBox* m_syntax     {nullptr};
    QCheckBox* m_changeBars {nullptr};
    QCheckBox* m_lineNums   {nullptr};
    QCheckBox*     m_modIndic   {nullptr};
    QSpinBox*      m_fontSize   {nullptr};
    QFontComboBox* m_fontFamily {nullptr};
    QSpinBox*      m_tabWidth   {nullptr};
    QCheckBox*     m_wordWrap   {nullptr};
    QCheckBox*     m_showWs     {nullptr};
    QCheckBox*     m_autoSave   {nullptr};
    QComboBox*     m_theme      {nullptr};
    QComboBox*     m_accent     {nullptr};
};

} // namespace zen::ui
