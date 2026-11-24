#pragma once

#include "Types.h"

#include <QByteArray>

namespace ps {

struct ShowFileResult {
    bool ok = false;
    Show show;
    QString error;
};

// FreeShow on-disk format: JSON array [showId, ShowObject].
QByteArray encodeShowFile(const Show& show);

ShowFileResult decodeShowFile(const QByteArray& bytes);

// A tolerant reader for truncated/corrupted files: tries to repair the JSON by
// closing open brackets, mirroring FreeShow's recovery behaviour.
ShowFileResult decodeShowFileRepair(const QByteArray& bytes);

// Disk layout helper: "<dataDir>/Shows/<sanitized name>.show"
QString showFilePath(const QString& dataDir, const QString& name);

QString sanitizeFileName(const QString& name);

} // namespace ps
