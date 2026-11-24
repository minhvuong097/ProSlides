#include "ChordEngine.h"

#include <QRegularExpression>
#include <QMap>

namespace ps::chords {

namespace {

int letterSemitone(const QChar letter)
{
    switch (letter.toLower().unicode()) {
    case u'c': return 0;
    case u'd': return 2;
    case u'e': return 4;
    case u'f': return 5;
    case u'g': return 7;
    case u'a': return 9;
    case u'b': return 11;
    default: return -1;
    }
}

QStringList sharpNames() { return { QStringLiteral(u"C"), QStringLiteral(u"C#"), QStringLiteral(u"D"), QStringLiteral(u"D#"), QStringLiteral(u"E"), QStringLiteral(u"F"), QStringLiteral(u"F#"), QStringLiteral(u"G"), QStringLiteral(u"G#"), QStringLiteral(u"A"), QStringLiteral(u"A#"), QStringLiteral(u"B")}; }
QStringList flatNames()  { return { QStringLiteral(u"C"), QStringLiteral(u"Db"), QStringLiteral(u"D"), QStringLiteral(u"Eb"), QStringLiteral(u"E"), QStringLiteral(u"F"), QStringLiteral(u"Gb"), QStringLiteral(u"G"), QStringLiteral(u"Ab"), QStringLiteral(u"A"), QStringLiteral(u"Bb"), QStringLiteral(u"B")}; }

// Match the leading root of a chord: note + optional accidental.
QRegularExpression rootPattern()
{
    static const QRegularExpression p(QStringLiteral(u"^([A-Ga-g])([#b]?)"));
    return p;
}

} // namespace

int rootToSemitone(const QString& root)
{
    const auto match = rootPattern().match(root);
    if (!match.hasMatch()) return -1;
    int value = letterSemitone(match.captured(1).at(0));
    if (value < 0) return -1;
    const QString accidental = match.captured(2);
    if (accidental == QStringLiteral(u"#")) ++value;
    else if (accidental == QStringLiteral(u"b")) --value;
    return (value % 12 + 12) % 12;
}

QString semitoneToRoot(int semitone, bool preferFlats)
{
    const int normalized = (semitone % 12 + 12) % 12;
    return (preferFlats ? flatNames() : sharpNames())[normalized];
}

QString canonicalRoot(const QString& root, bool preferFlats)
{
    const int value = rootToSemitone(root);
    return value < 0 ? root : semitoneToRoot(value, preferFlats);
}

QString transpose(const QString& chord, int semitones, bool preferFlats)
{
    const auto match = rootPattern().match(chord);
    if (!match.hasMatch()) return chord;
    const int rootLength = match.captured(0).size();
    const int value = rootToSemitone(match.captured(0));
    if (value < 0) return chord;
    const QString newRoot = semitoneToRoot(value + semitones, preferFlats);
    return newRoot + chord.mid(rootLength);
}

void transposeShow(Show& show, int semitones, bool preferFlats)
{
    for (auto& [id, slide] : show.slides) {
        Q_UNUSED(id);
        for (auto& item : slide.items) {
            for (auto& line : item.lines) {
                for (auto& chord : line.chords)
                    chord.key = transpose(chord.key, semitones, preferFlats);
            }
        }
        if (slide.key)
            slide.key = transpose(*slide.key, semitones, preferFlats);
    }
    if (show.meta.key)
        show.meta.key = transpose(*show.meta.key, semitones, preferFlats);
}

QString detectKey(const Show& show)
{
    QMap<int, int> counts;
    for (const auto& [id, slide] : show.slides) {
        Q_UNUSED(id);
        for (const auto& item : slide.items) {
            for (const auto& line : item.lines) {
                for (const auto& chord : line.chords) {
                    const int value = rootToSemitone(chord.key);
                    if (value >= 0) ++counts[value];
                }
            }
        }
    }
    if (counts.isEmpty()) return {};

    int bestRoot = 0;
    int bestCount = -1;
    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        if (it.value() > bestCount || (it.value() == bestCount && it.key() < bestRoot)) {
            bestCount = it.value();
            bestRoot = it.key();
        }
    }
    return semitoneToRoot(bestRoot);
}

} // namespace ps::chords
