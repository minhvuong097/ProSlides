#include "ShowFile.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>

namespace ps {

QByteArray encodeShowFile(const Show& show)
{
    QJsonArray root;
    root.append(show.id.isEmpty() ? makeId() : show.id);
    root.append(show.toJson());
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

ShowFileResult decodeShowFile(const QByteArray& bytes)
{
    ShowFileResult result;
    QJsonParseError err;
    const auto doc = QJsonDocument::fromJson(bytes, &err);
    if (err.error != QJsonParseError::NoError) {
        result.error = err.errorString();
        return result;
    }
    if (!doc.isArray() || doc.array().size() != 2 || !doc.array().at(1).isObject()) {
        result.error = u"Invalid .show file: expected [id, Show] tuple";
        return result;
    }
    const auto arr = doc.array();
    result.show = Show::fromJson(arr.at(1).toObject());
    result.show.id = arr.at(0).toString(result.show.id);
    if (result.show.id.isEmpty()) result.show.id = makeId();
    if (result.show.settings.activeLayout.isEmpty() && !result.show.layouts.empty())
        result.show.settings.activeLayout = result.show.layouts.begin()->first;
    result.ok = true;
    return result;
}

ShowFileResult decodeShowFileRepair(const QByteArray& bytes)
{
    auto result = decodeShowFile(bytes);
    if (result.ok) return result;

    // Try progressively trimming trailing garbage and closing brackets.
    QByteArray candidate = bytes.trimmed();
    for (int attempt = 0; attempt < 8 && !candidate.isEmpty(); ++attempt) {
        int braces = 0, brackets = 0;
        bool inString = false, escaped = false;
        for (const char ch : candidate) {
            if (escaped) { escaped = false; continue; }
            if (ch == '\\' && inString) { escaped = true; continue; }
            if (ch == '"') { inString = !inString; continue; }
            if (inString) continue;
            if (ch == '{') ++braces;
            else if (ch == '}') --braces;
            else if (ch == '[') ++brackets;
            else if (ch == ']') --brackets;
        }
        if (inString) candidate.chop(1);
        while (brackets-- > 0) candidate.append(']');
        while (braces-- > 0) candidate.append('}');

        result = decodeShowFile(candidate);
        if (result.ok) return result;

        // Drop the last line and retry.
        const int nl = candidate.lastIndexOf('\n');
        if (nl <= 0) break;
        candidate.truncate(nl);
    }
    result.error = u"Unable to repair .show file";
    return result;
}

QString sanitizeFileName(const QString& name)
{
    QString s = name.trimmed();
    for (const QChar forbidden : { '/', '\\', ':', '*', '?', '"', '<', '>', '|' })
        s.replace(forbidden, u'-');
    while (s.contains(u"--")) s.replace(u"--", u"-");
    if (s.isEmpty()) s = u"Untitled";
    return s;
}

QString showFilePath(const QString& dataDir, const QString& name)
{
    return QDir(dataDir).filePath(sanitizeFileName(name) + u".show");
}

} // namespace ps
