#include "Bible.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>

namespace ps::scripture {

QJsonObject Bible::toJson() const
{
    QJsonObject root;
    root.insert(QStringLiteral(u"name"), name);
    if (!copyright.isEmpty()) root.insert(QStringLiteral(u"copyright"), copyright);
    if (!metadata.isEmpty()) root.insert(QStringLiteral(u"metadata"), metadata);

    QJsonArray booksArr;
    for (const auto& book : books) {
        QJsonObject bo;
        bo.insert(QStringLiteral(u"number"), book.number);
        bo.insert(QStringLiteral(u"name"), book.name);
        if (!book.customName.isEmpty()) bo.insert(QStringLiteral(u"customName"), book.customName);
        if (!book.abbreviation.isEmpty()) bo.insert(QStringLiteral(u"abbreviation"), book.abbreviation);
        QJsonArray chaptersArr;
        for (const auto& chapter : book.chapters) {
            QJsonObject co;
            co.insert(QStringLiteral(u"number"), chapter.number);
            QJsonArray versesArr;
            for (const auto& verse : chapter.verses) {
                QJsonObject vo;
                vo.insert(QStringLiteral(u"number"), verse.number);
                if (!verse.value.isEmpty()) vo.insert(QStringLiteral(u"value"), verse.value);
                vo.insert(QStringLiteral(u"text"), verse.text);
                versesArr.append(vo);
            }
            co.insert(QStringLiteral(u"verses"), versesArr);
            chaptersArr.append(co);
        }
        bo.insert(QStringLiteral(u"chapters"), chaptersArr);
        booksArr.append(bo);
    }
    root.insert(QStringLiteral(u"books"), booksArr);
    return root;
}

Bible Bible::fromJson(const QJsonObject& o)
{
    Bible b;
    b.name = o.value(QStringLiteral(u"name")).toString();
    b.copyright = o.value(QStringLiteral(u"copyright")).toString();
    if (const auto v = o.constFind(QStringLiteral(u"metadata")); v != o.constEnd() && v->isObject())
        b.metadata = v->toObject();

    const auto arr = o.value(QStringLiteral(u"books")).toArray();
    for (const auto& bv : arr) {
        if (!bv.isObject()) continue;
        const auto bo = bv.toObject();
        Book book;
        book.number = bo.value(QStringLiteral(u"number")).toInt();
        book.name = bo.value(QStringLiteral(u"name")).toString();
        book.customName = bo.value(QStringLiteral(u"customName")).toString();
        book.abbreviation = bo.value(QStringLiteral(u"abbreviation")).toString();
        const auto chapters = bo.value(QStringLiteral(u"chapters")).toArray();
        for (const auto& cv : chapters) {
            if (!cv.isObject()) continue;
            const auto co = cv.toObject();
            Chapter chapter;
            chapter.number = co.value(QStringLiteral(u"number")).toInt();
            const auto verses = co.value(QStringLiteral(u"verses")).toArray();
            for (const auto& vv : verses) {
                if (!vv.isObject()) continue;
                const auto vo = vv.toObject();
                Verse verse;
                verse.number = vo.value(QStringLiteral(u"number")).toInt();
                verse.value = vo.value(QStringLiteral(u"value")).toString();
                verse.text = vo.value(QStringLiteral(u"text")).toString();
                chapter.verses.append(verse);
            }
            book.chapters.append(chapter);
        }
        b.books.append(book);
    }
    return b;
}

QString normalizeReferenceText(const QString& text)
{
    QString s = text.normalized(QString::NormalizationForm_D).toLower();
    s.replace(u'\u0111', u'd');
    QString out;
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s[i];
        if (c.category() != QChar::Mark_NonSpacing &&
            c.category() != QChar::Mark_SpacingCombining &&
            c.isLetterOrNumber()) out += c;
    }
    return out;
}

ScriptureRef parseReference(const QString& reference, const Bible& bible)
{
    ScriptureRef ref;
    const QString needle = normalizeReferenceText(reference);
    if (needle.isEmpty()) return ref;

    // Locate the book by full name, custom name or abbreviation (longest first).
    int bookNumber = 0;
    int consumed = 0;
    for (const auto& book : bible.books) {
        for (const QString& candidate : { book.customName, book.name, book.abbreviation }) {
            if (candidate.isEmpty()) continue;
            const QString key = normalizeReferenceText(candidate);
            if (needle.startsWith(key) && key.size() > consumed) {
                consumed = key.size();
                bookNumber = book.number;
            }
        }
    }
    if (bookNumber == 0) return ref;

    const QString tail = needle.mid(consumed).trimmed();
    static const QRegularExpression pattern(QStringLiteral(u"^(\\d+)\\s*:\\s*(\\d+)(?:\\s*-\\s*(\\d+))?$"));
    const auto match = pattern.match(tail);
    if (!match.hasMatch()) return ref;

    ref.book = bookNumber;
    ref.chapter = match.captured(1).toInt();
    ref.verse = match.captured(2).toInt();
    ref.endVerse = match.captured(3).isEmpty() ? 0 : match.captured(3).toInt();
    ref.valid = ref.chapter > 0 && ref.verse > 0;
    return ref;
}

QString referenceToString(const ScriptureRef& ref, const Bible& bible)
{
    for (const auto& book : bible.books) {
        if (book.number != ref.book) continue;
        const QString name = book.customName.isEmpty() ? book.name : book.customName;
        const QString range = ref.endVerse > 0
            ? QString::number(ref.verse) + QStringLiteral(u"-")+ QString::number(ref.endVerse)
            : QString::number(ref.verse);
        return name + QStringLiteral(u" ")+ QString::number(ref.chapter) + QStringLiteral(u":")+ range;
    }
    return {};
}

QByteArray encodeBibleFile(const Bible& bible)
{
    QJsonArray root;
    root.append(bible.id.isEmpty() ? makeId() : bible.id);
    root.append(bible.toJson());
    return QJsonDocument(root).toJson(QJsonDocument::Indented);
}

BibleFileResult decodeBibleFile(const QByteArray& bytes)
{
    BibleFileResult result;
    const auto doc = QJsonDocument::fromJson(bytes);
    if (!doc.isArray() || doc.array().size() != 2 || !doc.array().at(1).isObject()) {
        result.error = QStringLiteral(u"Invalid .fsb file");
        return result;
    }
    result.bible = Bible::fromJson(doc.array().at(1).toObject());
    result.bible.id = doc.array().at(0).toString(result.bible.id);
    result.ok = true;
    return result;
}

} // namespace ps::scripture
