#pragma once

#include <QRegularExpression>
#include <QString>
#include <QVector>

namespace zen::syntax {

enum class TokenKind {
    Keyword,
    Type,
    Preprocessor,
    Number,
    String,
    Comment,
    Function,
    Operator,
    Punctuation,
};

struct HighlightRule {
    QRegularExpression pattern;
    TokenKind          kind;
};

struct LanguageRules {
    QString              name;
    QVector<HighlightRule> rules;
    // Multi-line block comment delimiters (empty when unused).
    QRegularExpression   blockCommentStart;
    QRegularExpression   blockCommentEnd;
};

// Selects a language for the given file path (extension-based); returns the
// `plain` set when no specific language matches.
const LanguageRules& rulesForPath(const QString& path);

// Named language lookup: "cpp", "python", "json", "markdown", "plain".
const LanguageRules& rulesByName(const QString& name);

} // namespace zen::syntax
