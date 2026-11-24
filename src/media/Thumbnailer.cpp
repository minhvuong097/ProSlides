#include "Thumbnailer.h"

#include <QImageReader>
#include <QFileIconProvider>
#include <QFileInfo>

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
        QFileIconProvider icons;
        image = icons.icon(QFileInfo(path)).pixmap(m_size).toImage();
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
