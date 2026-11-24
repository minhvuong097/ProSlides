#pragma once

#include <QObject>
#include <QImage>
#include <QSize>
#include <QMap>

namespace ps::media {

// Generates and caches slide/media thumbnails off the UI thread.
class Thumbnailer : public QObject
{
    Q_OBJECT
public:
    explicit Thumbnailer(QObject* parent = nullptr);

    void setThumbnailSize(const QSize& size) { m_size = size; }
    QImage cached(const QString& path) const;
    QImage thumbnail(const QString& path);

public slots:
    void clearCache();

signals:
    void ready(const QString& path, const QImage& image);

private:
    QSize m_size = { 320, 180 };
    QMap<QString, QImage> m_cache;
};

} // namespace ps::media
