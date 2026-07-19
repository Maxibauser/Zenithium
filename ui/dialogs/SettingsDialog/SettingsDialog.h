#pragma once

#include <QDialog>

class QCheckBox;
class QComboBox;
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
    QCheckBox* m_modIndic   {nullptr};
    QSpinBox*  m_fontSize   {nullptr};
    QComboBox* m_theme      {nullptr};
    QComboBox* m_accent     {nullptr};
};

} // namespace zen::ui
