#pragma once

#include "Bible.h"

#include <QObject>
#include <QVariantList>
#include <QStringList>

namespace ps {
class Database;
}

namespace ps::scripture {

// Loads FreeShow-compatible .fsb Bibles and provides reference parsing,
// passage lookup and full-text scripture search.
class ScriptureProvider : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList bibleNames READ bibleNames NOTIFY biblesChanged)
    Q_PROPERTY(QString activeReference READ activeReference NOTIFY activeReferenceChanged)
    Q_PROPERTY(QString activeText READ activeText NOTIFY activeReferenceChanged)

public:
    explicit ScriptureProvider(Database* database, QObject* parent = nullptr);

    void setDirectory(const QString& dir) { m_directory = dir; }
    QString directory() const { return m_directory; }

    QStringList bibleNames() const;
    QString activeReference() const { return m_activeReference; }
    QString activeText() const { return m_activeText; }
    QString activeBibleId() const { return m_activeBibleId; }

    QVariantList books() const;
    QVariantList search(const QString& query, int limit = 60) const;

    Q_INVOKABLE bool setActiveBible(const QString& id);
    Q_INVOKABLE bool openReference(const QString& reference);
    Q_INVOKABLE bool openNextVerse();
    Q_INVOKABLE bool openPreviousVerse();
    Q_INVOKABLE void load();

    void setActiveVerse(const ScriptureRef& ref);

signals:
    void biblesChanged();
    void activeReferenceChanged();
    void errorOccurred(const QString& error);

private:
    const Bible* activeBible() const;
    void indexBible(const Bible& bible);
    QString verseText(const ScriptureRef& ref) const;

    Database* m_database = nullptr;
    QString m_directory;
    QVector<Bible> m_bibles;
    QString m_activeBibleId;
    ScriptureRef m_activeRef;
    QString m_activeReference;
    QString m_activeText;
};

} // namespace ps::scripture
