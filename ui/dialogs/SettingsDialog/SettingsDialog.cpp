#include "SettingsDialog.h"

#include "Preferences.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace zen::ui {

namespace {

struct AccentSwatch {
    const char* name;
    const char* hex;
};
constexpr AccentSwatch kAccents[] = {
    {"Blue",   "#6ea8ff"},
    {"Purple", "#7c5cff"},
    {"Green",  "#2ea043"},
    {"Pink",   "#e879a1"},
    {"Amber",  "#d19a66"},
};

} // namespace

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setObjectName("ZenSettingsDialog");
    setWindowTitle(tr("Settings"));
    setModal(true);
    setMinimumWidth(460);

    auto* root = new QVBoxLayout(this);

    // --- Editor group ---
    auto* editorBox = new QGroupBox(tr("Editor"), this);
    auto* editor = new QFormLayout(editorBox);
    editor->setLabelAlignment(Qt::AlignLeft);

    m_syntax     = new QCheckBox(tr("Enable syntax highlighting"), editorBox);
    m_changeBars = new QCheckBox(tr("Show unsaved-changes bars in gutter"), editorBox);
    m_lineNums   = new QCheckBox(tr("Show line numbers"), editorBox);
    m_modIndic   = new QCheckBox(tr("Show modified indicator (● in tab / title)"), editorBox);
    m_fontSize   = new QSpinBox(editorBox);
    m_fontSize->setRange(8, 32);
    m_fontSize->setSuffix(tr(" pt"));

    editor->addRow(m_syntax);
    editor->addRow(m_changeBars);
    editor->addRow(m_lineNums);
    editor->addRow(m_modIndic);
    editor->addRow(tr("Font size"), m_fontSize);
    root->addWidget(editorBox);

    // --- Appearance group ---
    auto* appBox = new QGroupBox(tr("Appearance"), this);
    auto* app    = new QFormLayout(appBox);

    m_theme = new QComboBox(appBox);
    m_theme->addItem(tr("Dark"),  "dark");
    m_theme->addItem(tr("Light"), "light");

    m_accent = new QComboBox(appBox);
    for (const auto& a : kAccents) {
        QPixmap swatch(14, 14);
        swatch.fill(QColor(a.hex));
        m_accent->addItem(QIcon(swatch), tr(a.name), QString::fromUtf8(a.hex));
    }

    app->addRow(tr("Theme"),        m_theme);
    app->addRow(tr("Accent color"), m_accent);
    root->addWidget(appBox);

    // --- Buttons ---
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply, this);
    root->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, this, [this] {
        applyToPrefs();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::applyToPrefs);

    loadFromPrefs();
}

void SettingsDialog::loadFromPrefs() {
    const auto& p = Preferences::instance();
    m_syntax    ->setChecked(p.syntaxHighlightingEnabled());
    m_changeBars->setChecked(p.changeBarsEnabled());
    m_lineNums  ->setChecked(p.lineNumbersEnabled());
    m_modIndic  ->setChecked(p.modifiedIndicatorEnabled());
    m_fontSize  ->setValue(p.editorFontSize());

    const int themeIdx  = m_theme->findData(p.theme());
    m_theme ->setCurrentIndex(themeIdx  >= 0 ? themeIdx  : 0);
    const int accentIdx = m_accent->findData(p.accentColor().name());
    m_accent->setCurrentIndex(accentIdx >= 0 ? accentIdx : 0);
}

void SettingsDialog::applyToPrefs() {
    auto& p = Preferences::instance();
    p.setSyntaxHighlightingEnabled(m_syntax    ->isChecked());
    p.setChangeBarsEnabled        (m_changeBars->isChecked());
    p.setLineNumbersEnabled       (m_lineNums  ->isChecked());
    p.setModifiedIndicatorEnabled (m_modIndic  ->isChecked());
    p.setEditorFontSize           (m_fontSize  ->value());
    p.setTheme                    (m_theme     ->currentData().toString());
    p.setAccentColor              (QColor(m_accent->currentData().toString()));
}

} // namespace zen::ui
