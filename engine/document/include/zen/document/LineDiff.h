#pragma once

#include <QString>
#include <QVector>

namespace zen::document {

enum class LineChange : quint8 {
    Unchanged,
    Modified,   // this line differs from the saved snapshot
    Added,      // this line is beyond the end of the saved snapshot
};

// Returns a per-line change kind for `current`, using `saved` as the baseline.
// Uses a simple LCS on line hashes; O(N*M) time/memory — fine for typical
// source files, will be replaced with a chunked / incremental diff later.
QVector<LineChange> diffLines(const QString& saved, const QString& current);

} // namespace zen::document
