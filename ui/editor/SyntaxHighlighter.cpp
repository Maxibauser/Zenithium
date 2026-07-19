#include "SyntaxHighlighter.h"

#include <QColor>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>

namespace zen::ui {

using zen::syntax::TokenKind;

SyntaxHighlighter::SyntaxHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
    initFormats();
    setLanguage(zen::syntax::rulesByName(QStringLiteral("plain")));
}

void SyntaxHighlighter::initFormats() {
    // One Dark inspired palette
    auto set = [&](TokenKind k, const QColor& c, bool italic = false, bool bold = false) {
        QTextCharFormat f;
        f.setForeground(c);
        f.setFontItalic(italic);
        if (bold) f.setFontWeight(QFont::DemiBold);
        m_formats[static_cast<size_t>(k)] = f;
    };
    set(TokenKind::Keyword,      QColor(0xc6, 0x78, 0xdd), false, true);   // purple
    set(TokenKind::Type,         QColor(0xe5, 0xc0, 0x7b));                // yellow
    set(TokenKind::Preprocessor, QColor(0x56, 0xb6, 0xc2));                // cyan
    set(TokenKind::Number,       QColor(0xd1, 0x9a, 0x66));                // orange
    set(TokenKind::String,       QColor(0x98, 0xc3, 0x79));                // green
    set(TokenKind::Comment,      QColor(0x5c, 0x63, 0x70), true);          // gray, italic
    set(TokenKind::Function,     QColor(0x61, 0xaf, 0xef));                // blue
    set(TokenKind::Operator,     QColor(0xab, 0xb2, 0xbf));                // light gray
    set(TokenKind::Punctuation,  QColor(0xab, 0xb2, 0xbf));
}

void SyntaxHighlighter::setLanguage(const zen::syntax::LanguageRules& rules) {
    m_rules = &rules;
    rehighlight();
}

void SyntaxHighlighter::setLanguageForPath(const QString& path) {
    setLanguage(zen::syntax::rulesForPath(path));
}

void SyntaxHighlighter::highlightBlock(const QString& text) {
    if (!m_rules) return;

    for (const auto& r : m_rules->rules) {
        auto it = r.pattern.globalMatch(text);
        while (it.hasNext()) {
            const auto m = it.next();
            setFormat(static_cast<int>(m.capturedStart()),
                      static_cast<int>(m.capturedLength()),
                      m_formats[static_cast<size_t>(r.kind)]);
        }
    }

    // Multi-line block comments (state 1 = inside comment).
    if (!m_rules->blockCommentStart.pattern().isEmpty()) {
        setCurrentBlockState(0);
        int startIndex = 0;
        if (previousBlockState() != 1) {
            const auto m = m_rules->blockCommentStart.match(text);
            startIndex = m.hasMatch() ? static_cast<int>(m.capturedStart()) : -1;
        }
        while (startIndex >= 0) {
            const auto endMatch = m_rules->blockCommentEnd.match(text, startIndex);
            int length;
            if (!endMatch.hasMatch()) {
                setCurrentBlockState(1);
                length = text.length() - startIndex;
            } else {
                length = static_cast<int>(endMatch.capturedEnd()) - startIndex;
            }
            setFormat(startIndex, length,
                      m_formats[static_cast<size_t>(TokenKind::Comment)]);
            const auto next = m_rules->blockCommentStart.match(text, startIndex + length);
            startIndex = next.hasMatch() ? static_cast<int>(next.capturedStart()) : -1;
        }
    }
}

} // namespace zen::ui
