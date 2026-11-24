#include "Thumbnailer.h"

#include <QImageReader>
#include <QFileInfo>
#include <QColor>

namespace ps::media {

Thumbnailer::Thumbnailer(QObject* parent) : QObject(parent)
{
}

QImage Thumbnailer::cached(const QString& path) const
{
    return m_cache.value(path);
}

QImage Thumbnailer::thumbnail(const QString& path)
{
    if (const auto cached = cached(path); !cached.isNull()) return cached;

    QImage image;
    QImageReader reader(path);
    if (reader.canRead()) {
        reader.setScaledSize(reader.size().scaled(m_size, Qt::KeepAspectRatio));
        image = reader.read();
    }
    if (image.isNull()) {
        // Video/audio files: draw a neutral placeholder (no QtWidgets needed).
        image = QImage(m_size, QImage::Format_ARGB32);
        image.fill(QColor(u"#232329"));
    }
    if (!image.isNull()) m_cache.insert(path, image);
    emit ready(path, image);
    return image;
}

void Thumbnailer::clearCache()
{
    m_cache.clear();
}

} // namespace ps::media
