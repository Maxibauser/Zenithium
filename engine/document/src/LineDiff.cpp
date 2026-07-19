#include "zen/document/LineDiff.h"

#include <QStringView>
#include <vector>

namespace zen::document {

namespace {

QVector<QStringView> splitLines(const QString& text) {
    QVector<QStringView> lines;
    lines.reserve(text.count(QLatin1Char('\n')) + 1);
    int start = 0;
    const int n = text.size();
    for (int i = 0; i < n; ++i) {
        if (text[i] == QLatin1Char('\n')) {
            lines.append(QStringView(text).mid(start, i - start));
            start = i + 1;
        }
    }
    if (start <= n) {
        lines.append(QStringView(text).mid(start, n - start));
    }
    return lines;
}

} // namespace

QVector<LineChange> diffLines(const QString& saved, const QString& current) {
    const auto s = splitLines(saved);
    const auto c = splitLines(current);
    const int m = s.size();
    const int n = c.size();

    QVector<LineChange> result(n, LineChange::Modified);

    if (saved.isEmpty()) {
        for (auto& k : result) k = LineChange::Added;
        return result;
    }

    // LCS DP over line-view equality.
    std::vector<std::vector<int>> dp(static_cast<size_t>(m) + 1,
                                     std::vector<int>(static_cast<size_t>(n) + 1, 0));
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= n; ++j) {
            if (s[i - 1] == c[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // Traceback: mark unchanged lines, distinguish Added vs Modified.
    int i = m;
    int j = n;
    while (i > 0 && j > 0) {
        if (s[i - 1] == c[j - 1]) {
            result[j - 1] = LineChange::Unchanged;
            --i; --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i; // line deleted from saved
        } else {
            // Line inserted in current. If nothing was deleted immediately
            // "at" this position in saved, treat as pure Added; otherwise
            // Modified (a replacement).
            result[j - 1] = (i == 0) ? LineChange::Added : LineChange::Modified;
            --j;
        }
    }
    while (j > 0) {
        result[j - 1] = LineChange::Added;
        --j;
    }
    return result;
}

} // namespace zen::document
