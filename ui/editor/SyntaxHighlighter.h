#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include <array>

#include "zen/syntax/LanguageRules.h"

namespace zen::ui {

class SyntaxHighlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit SyntaxHighlighter(QTextDocument* parent = nullptr);

    void setLanguage(const zen::syntax::LanguageRules& rules);
    void setLanguageForPath(const QString& path);

protected:
    void highlightBlock(const QString& text) override;

private:
    void initFormats();

    const zen::syntax::LanguageRules* m_rules {nullptr};
    std::array<QTextCharFormat, 9>    m_formats;
};

} // namespace zen::ui
