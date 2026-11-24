#include "ScriptureProvider.h"
#include "persistence/Database.h"

#include <QDirIterator>
#include <QFile>
#include <QSqlQuery>
#include <QRegularExpression>
#include <QVariantMap>

namespace ps::scripture {

ScriptureProvider::ScriptureProvider(Database* database, QObject* parent)
    : QObject(parent), m_database(database)
{
}

const Bible* ScriptureProvider::activeBible() const
{
    for (const auto& bible : m_bibles) {
        if (bible.id == m_activeBibleId) return &bible;
    }
    return m_bibles.isEmpty() ? nullptr : &m_bibles.first();
}

QStringList ScriptureProvider::bibleNames() const
{
    QStringList out;
    for (const auto& bible : m_bibles) out << bible.name;
    return out;
}

void ScriptureProvider::load()
{
    m_bibles.clear();
    if (m_directory.isEmpty()) { emit biblesChanged(); return; }

    QDirIterator it(m_directory, { QStringLiteral(u"*.fsb")}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QFile file(it.next());
        if (!file.open(QIODevice::ReadOnly)) continue;
        const auto result = decodeBibleFile(file.readAll());
        file.close();
        if (!result.ok) {
            emit errorOccurred(it.filePath() + QStringLiteral(u": ")+ result.error);
            continue;
        }
        m_bibles.append(result.bible);
        indexBible(result.bible);
    }
    if (!m_bibles.isEmpty() && activeBible() == nullptr)
        m_activeBibleId = m_bibles.first().id;
    emit biblesChanged();
}

void ScriptureProvider::indexBible(const Bible& bible)
{
    if (!m_database || !m_database->isOpen()) return;
    QSqlQuery del(m_database->database());
    del.prepare(QStringLiteral(u"DELETE FROM verses_fts WHERE bible_id = :id"));
    del.bindValue(QStringLiteral(u":id"), bible.id);
    del.exec();

    QSqlQuery insert(m_database->database());
    insert.prepare(QStringLiteral(u"INSERT INTO verses_fts (bible_id, book, chapter, verse, text) "
                   u"VALUES (:bid, :book, :chapter, :verse, :text)"));
    insert.bindValue(QStringLiteral(u":bid"), bible.id);
    for (const auto& book : bible.books) {
        insert.bindValue(QStringLiteral(u":book"), book.name);
        for (const auto& chapter : book.chapters) {
            insert.bindValue(QStringLiteral(u":chapter"), chapter.number);
            for (const auto& verse : chapter.verses) {
                insert.bindValue(QStringLiteral(u":verse"), verse.number);
                insert.bindValue(QStringLiteral(u":text"), verse.text);
                insert.exec();
            }
        }
    }
}

QVariantList ScriptureProvider::books() const
{
    QVariantList out;
    const Bible* bible = activeBible();
    if (!bible) return out;
    for (const auto& book : bible->books) {
        QVariantMap m;
        m.insert(QStringLiteral(u"number"), book.number);
        m.insert(QStringLiteral(u"name"), book.customName.isEmpty() ? book.name : book.customName);
        m.insert(QStringLiteral(u"abbreviation"), book.abbreviation);
        m.insert(QStringLiteral(u"chapters"), book.chapters.size());
        out.append(m);
    }
    return out;
}

QVariantList ScriptureProvider::search(const QString& query, int limit) const
{
    QVariantList out;
    const Bible* bible = activeBible();
    if (!bible) return out;

    const QString needle = normalizeReferenceText(query);
    const auto ref = parseReference(query, *bible);
    if (ref.valid) {
        // A parsed reference jumps straight to the passage.
        QVariantMap m;
        m.insert(QStringLiteral(u"reference"), referenceToString(ref, *bible));
        m.insert(QStringLiteral(u"text"), verseText(ref));
        m.insert(QStringLiteral(u"book"), ref.book);
        m.insert(QStringLiteral(u"chapter"), ref.chapter);
        m.insert(QStringLiteral(u"verse"), ref.verse);
        out.append(m);
        return out;
    }

    const auto words = needle.split(QRegularExpression(QStringLiteral(u"\\s+")), Qt::SkipEmptyParts);
    if (words.isEmpty()) return out;

    for (const auto& book : bible->books) {
        for (const auto& chapter : book.chapters) {
            for (const auto& verse : chapter.verses) {
                const QString haystack = normalizeReferenceText(verse.text);
                bool matches = true;
                for (const auto& word : words) {
                    if (!haystack.contains(word)) { matches = false; break; }
                }
                if (!matches) continue;

                ScriptureRef hit;
                hit.book = book.number;
                hit.chapter = chapter.number;
                hit.verse = verse.number;
                QVariantMap m;
                m.insert(QStringLiteral(u"reference"), referenceToString(hit, *bible));
                m.insert(QStringLiteral(u"text"), verse.text);
                m.insert(QStringLiteral(u"book"), book.number);
                m.insert(QStringLiteral(u"chapter"), chapter.number);
                m.insert(QStringLiteral(u"verse"), verse.number);
                out.append(m);
                if (out.size() >= limit) return out;
            }
        }
    }
    return out;
}

bool ScriptureProvider::setActiveBible(const QString& id)
{
    QString resolved;
    for (const auto& bible : m_bibles) {
        if (bible.id == id || bible.name == id) { resolved = bible.id; break; }
    }
    if (resolved.isEmpty()) return false;
    m_activeBibleId = resolved;
    emit activeReferenceChanged();
    return true;
}

QString ScriptureProvider::verseText(const ScriptureRef& ref) const
{
    const Bible* bible = activeBible();
    if (!bible) return {};
    QStringList verses;
    for (const auto& book : bible->books) {
        if (book.number != ref.book) continue;
        for (const auto& chapter : book.chapters) {
            if (chapter.number != ref.chapter) continue;
            const int end = ref.endVerse > 0 ? ref.endVerse : ref.verse;
            for (const auto& verse : chapter.verses) {
                if (verse.number >= ref.verse && verse.number <= end)
                    verses << verse.text;
            }
        }
    }
    return verses.join(u' ');
}

void ScriptureProvider::setActiveVerse(const ScriptureRef& ref)
{
    m_activeRef = ref;
    m_activeReference = referenceToString(ref, *activeBible());
    m_activeText = verseText(ref);
    emit activeReferenceChanged();
}

bool ScriptureProvider::openReference(const QString& reference)
{
    const Bible* bible = activeBible();
    if (!bible) return false;
    const ScriptureRef ref = parseReference(reference, *bible);
    if (!ref.valid) return false;
    setActiveVerse(ref);
    return true;
}

bool ScriptureProvider::openNextVerse()
{
    const Bible* bible = activeBible();
    if (!bible || !m_activeRef.valid) return false;
    ScriptureRef ref = m_activeRef;
    ref.endVerse = 0;

    for (const auto& book : bible->books) {
        if (book.number != ref.book) continue;
        for (int ci = 0; ci < book.chapters.size(); ++ci) {
            const auto& chapter = book.chapters[size_t(ci)];
            if (chapter.number != ref.chapter) continue;
            for (int vi = 0; vi < chapter.verses.size(); ++vi) {
                if (chapter.verses[size_t(vi)].number != ref.verse) continue;
                if (vi + 1 < chapter.verses.size()) {
                    ref.verse = chapter.verses[size_t(vi + 1)].number;
                } else if (ci + 1 < book.chapters.size() &&
                           !book.chapters[size_t(ci + 1)].verses.isEmpty()) {
                    ref.chapter = book.chapters[size_t(ci + 1)].number;
                    ref.verse = book.chapters[size_t(ci + 1)].verses.first().number;
                }
                setActiveVerse(ref);
                return true;
            }
            return false;
        }
    }
    return false;
}

bool ScriptureProvider::openPreviousVerse()
{
    if (!activeBible() || !m_activeRef.valid) return false;
    ScriptureRef ref = m_activeRef;
    ref.endVerse = 0;
    ref.verse = qMax(1, ref.verse - 1);
    if (ref.verse == 1 && ref.chapter > 1) --ref.chapter;
    setActiveVerse(ref);
    return true;
}

} // namespace ps::scripture
