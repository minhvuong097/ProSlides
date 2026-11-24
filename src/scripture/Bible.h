#pragma once

#include "Types.h"

#include <QJsonObject>
#include <QVector>
#include <QMap>

namespace ps::scripture {

// FreeShow Bible.ts compatible structures. Files on disk are [id, Bible].
struct Verse {
    int number = 0;
    QString value;   // FreeShow: alternate numbering label (optional)
    QString text;
};
struct Chapter {
    int number = 0;
    QVector<Verse> verses;
};
struct Book {
    int number = 0;
    QString name;
    QString customName;
    QString abbreviation;
    QVector<Chapter> chapters;
};
struct Bible {
    Id id;
    QString name;
    QString copyright;
    QJsonObject metadata;
    QVector<Book> books;

    QJsonObject toJson() const;
    static Bible fromJson(const QJsonObject& o);
};

struct ScriptureRef {
    int book = 0;      // 1-based book number
    int chapter = 1;
    int verse = 1;
    int endVerse = 0;  // 0 = single verse
    bool valid = false;
};

QString normalizeReferenceText(const QString& text);
ScriptureRef parseReference(const QString& reference, const Bible& bible);
QString referenceToString(const ScriptureRef& ref, const Bible& bible);

QByteArray encodeBibleFile(const Bible& bible);
struct BibleFileResult { bool ok = false; Bible bible; QString error; };
BibleFileResult decodeBibleFile(const QByteArray& bytes);

} // namespace ps::scripture
