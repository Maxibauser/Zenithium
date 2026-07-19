#include "zen/syntax/LanguageRules.h"

#include <QFileInfo>
#include <QHash>

namespace zen::syntax {

namespace {

HighlightRule rule(const QString& pattern, TokenKind kind) {
    QRegularExpression re(pattern);
    re.optimize();
    return {re, kind};
}

QString keywordPattern(const QStringList& words) {
    return QStringLiteral("\\b(?:%1)\\b").arg(words.join(QLatin1Char('|')));
}

LanguageRules makePlain() {
    return {QStringLiteral("plain"), {}, {}, {}};
}

LanguageRules makeCpp() {
    LanguageRules L;
    L.name = QStringLiteral("cpp");

    static const QStringList keywords = {
        "alignas","alignof","asm","auto","break","case","catch","class","const",
        "consteval","constexpr","constinit","const_cast","continue","co_await",
        "co_return","co_yield","decltype","default","delete","do","dynamic_cast",
        "else","enum","explicit","export","extern","false","final","for","friend",
        "goto","if","inline","mutable","namespace","new","noexcept","nullptr",
        "operator","override","private","protected","public","reinterpret_cast",
        "return","sizeof","static","static_assert","static_cast","struct",
        "switch","template","this","thread_local","throw","true","try","typedef",
        "typeid","typename","union","using","virtual","volatile","while"
    };
    static const QStringList types = {
        "bool","char","char8_t","char16_t","char32_t","double","float","int",
        "long","short","signed","unsigned","void","wchar_t",
        "int8_t","int16_t","int32_t","int64_t","uint8_t","uint16_t","uint32_t",
        "uint64_t","size_t","ptrdiff_t","intptr_t","uintptr_t"
    };

    L.rules = {
        rule(QStringLiteral("^\\s*#\\s*\\w+"),                    TokenKind::Preprocessor),
        rule(keywordPattern(keywords),                             TokenKind::Keyword),
        rule(keywordPattern(types),                                TokenKind::Type),
        rule(QStringLiteral("\\b[A-Z][A-Za-z0-9_]*\\b"),           TokenKind::Type),
        rule(QStringLiteral("\\b\\d+(?:\\.\\d+)?(?:[eE][-+]?\\d+)?[uUlLfF]*\\b"),
             TokenKind::Number),
        rule(QStringLiteral("\\b0x[0-9a-fA-F]+[uUlL]*\\b"),        TokenKind::Number),
        rule(QStringLiteral("R\"\\(.*\\)\""),                      TokenKind::String),
        rule(QStringLiteral("\"(?:\\\\.|[^\"\\\\])*\""),           TokenKind::String),
        rule(QStringLiteral("'(?:\\\\.|[^'\\\\])'"),               TokenKind::String),
        rule(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()"), TokenKind::Function),
        rule(QStringLiteral("//[^\n]*"),                           TokenKind::Comment),
    };
    L.blockCommentStart = QRegularExpression(QStringLiteral("/\\*"));
    L.blockCommentEnd   = QRegularExpression(QStringLiteral("\\*/"));
    return L;
}

LanguageRules makePython() {
    LanguageRules L;
    L.name = QStringLiteral("python");
    static const QStringList keywords = {
        "False","None","True","and","as","assert","async","await","break","class",
        "continue","def","del","elif","else","except","finally","for","from",
        "global","if","import","in","is","lambda","nonlocal","not","or","pass",
        "raise","return","try","while","with","yield","match","case"
    };
    static const QStringList builtins = {
        "int","float","str","bool","list","dict","tuple","set","bytes","bytearray",
        "print","len","range","open","type","isinstance","enumerate","zip","map",
        "filter","sorted","sum","min","max","abs","round"
    };
    L.rules = {
        rule(keywordPattern(keywords), TokenKind::Keyword),
        rule(keywordPattern(builtins), TokenKind::Type),
        rule(QStringLiteral("\\b\\d+(?:\\.\\d+)?(?:[eE][-+]?\\d+)?\\b"), TokenKind::Number),
        rule(QStringLiteral("\"(?:\\\\.|[^\"\\\\])*\""),                TokenKind::String),
        rule(QStringLiteral("'(?:\\\\.|[^'\\\\])*'"),                   TokenKind::String),
        rule(QStringLiteral("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()"),    TokenKind::Function),
        rule(QStringLiteral("#[^\n]*"),                                 TokenKind::Comment),
    };
    return L;
}

LanguageRules makeJson() {
    LanguageRules L;
    L.name = QStringLiteral("json");
    L.rules = {
        rule(QStringLiteral("\"(?:\\\\.|[^\"\\\\])*\"\\s*(?=:)"),       TokenKind::Function),
        rule(QStringLiteral("\"(?:\\\\.|[^\"\\\\])*\""),                TokenKind::String),
        rule(QStringLiteral("\\b(?:true|false|null)\\b"),               TokenKind::Keyword),
        rule(QStringLiteral("-?\\b\\d+(?:\\.\\d+)?(?:[eE][-+]?\\d+)?\\b"), TokenKind::Number),
    };
    return L;
}

LanguageRules makeMarkdown() {
    LanguageRules L;
    L.name = QStringLiteral("markdown");
    L.rules = {
        rule(QStringLiteral("^#{1,6} .+$"),        TokenKind::Keyword),
        rule(QStringLiteral("\\*\\*[^*]+\\*\\*"),  TokenKind::Type),
        rule(QStringLiteral("`[^`]+`"),            TokenKind::String),
        rule(QStringLiteral("\\[[^\\]]+\\]\\([^)]+\\)"), TokenKind::Function),
        rule(QStringLiteral("^>.*$"),              TokenKind::Comment),
    };
    return L;
}

struct Catalog {
    LanguageRules plain    = makePlain();
    LanguageRules cpp      = makeCpp();
    LanguageRules python   = makePython();
    LanguageRules json     = makeJson();
    LanguageRules markdown = makeMarkdown();
};

const Catalog& catalog() {
    static const Catalog c;
    return c;
}

} // namespace

const LanguageRules& rulesByName(const QString& name) {
    const auto& c = catalog();
    if (name == QLatin1String("cpp"))      return c.cpp;
    if (name == QLatin1String("python"))   return c.python;
    if (name == QLatin1String("json"))     return c.json;
    if (name == QLatin1String("markdown")) return c.markdown;
    return c.plain;
}

const LanguageRules& rulesForPath(const QString& path) {
    const QString ext = QFileInfo(path).suffix().toLower();
    static const QHash<QString, QString> table = {
        {"c","cpp"},{"h","cpp"},{"hh","cpp"},{"hpp","cpp"},
        {"cc","cpp"},{"cpp","cpp"},{"cxx","cpp"},{"ipp","cpp"},{"inl","cpp"},
        {"py","python"},{"pyi","python"},{"pyw","python"},
        {"json","json"},
        {"md","markdown"},{"markdown","markdown"},
    };
    return rulesByName(table.value(ext, QStringLiteral("plain")));
}

} // namespace zen::syntax
