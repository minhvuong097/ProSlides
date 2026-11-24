#pragma once

#include "Types.h"

#include <QString>
#include <QStringList>

namespace ps::chords {

// Transpose a single chord symbol by semitones (e.g. "G/B" +2 -> "A/C#").
// Supports sharps and flats: C# / Db, with optional preference for flat names.
QString transpose(const QString& chord, int semitones, bool preferFlats = false);

// Rotate the whole show's chords and key metadata.
// Positive semitones = up, negative = down.
void transposeShow(Show& show, int semitones, bool preferFlats = false);

// Guess the show's key from chord-root frequency. Returns e.g. "C" or "G".
QString detectKey(const Show& show);

// Normalize a chord root to a canonical sharp/flat spelling.
QString canonicalRoot(const QString& root, bool preferFlats = false);

int rootToSemitone(const QString& root);
QString semitoneToRoot(int semitone, bool preferFlats = false);

} // namespace ps::chords
