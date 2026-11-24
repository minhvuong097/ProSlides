#pragma once

#include "Types.h"

#include <QObject>
#include <QVector>
#include <QDateTime>

namespace ps::media {

struct MediaFile {
    QString path;
    QString name;
    MediaType type = MediaType::Media;
    qint64 size = 0;
    QDateTime modified;
};

// Scans a media folder into an in-memory index for the drag-and-drop library.
class MediaLibrary : public QObject
{
    Q_OBJECT
public:
    explicit MediaLibrary(QObject* parent = nullptr);

    void setRoot(const QString& path) { m_root = path; }
    QString root() const { return m_root; }

    QVector<MediaFile> files() const { return m_files; }
    QVector<MediaFile> search(const QString& query) const;

public slots:
    void scan();

signals:
    void scanned(int count);
    void fileAdded(const MediaFile& file);

private:
    static MediaType typeForFile(const QString& path);

    QString m_root;
    QVector<MediaFile> m_files;
};

} // namespace ps::media
