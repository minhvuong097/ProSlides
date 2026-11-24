#include "MediaLibrary.h"

#include <QDirIterator>
#include <QFileInfo>

namespace ps::media {

MediaLibrary::MediaLibrary(QObject* parent) : QObject(parent)
{
}

MediaType MediaLibrary::typeForFile(const QString& path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    static const QStringList images = { u"png", u"jpg", u"jpeg", u"webp", u"bmp", u"gif", u"svg", u"tif", u"tiff" };
    static const QStringList videos = { u"mp4", u"mov", u"mkv", u"avi", u"webm", u"m4v", u"wmv", u"mpg", u"mpeg" };
    static const QStringList audios = { u"mp3", u"wav", u"aac", u"flac", u"ogg", u"m4a", u"wma" };
    if (images.contains(suffix)) return MediaType::Image;
    if (videos.contains(suffix)) return MediaType::Video;
    if (audios.contains(suffix)) return MediaType::Audio;
    return MediaType::Media;
}

void MediaLibrary::scan()
{
    m_files.clear();
    if (m_root.isEmpty() || !QFileInfo(m_root).exists()) {
        emit scanned(0);
        return;
    }

    QDirIterator it(m_root, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = it.next();
        const QFileInfo info(path);
        if (!info.isFile()) continue;
        const auto type = typeForFile(path);
        if (type == MediaType::Media) continue;

        MediaFile file;
        file.path = path;
        file.name = info.completeBaseName();
        file.type = type;
        file.size = info.size();
        file.modified = info.lastModified();
        m_files << file;
        emit fileAdded(file);
    }
    emit scanned(m_files.size());
}

QVector<MediaFile> MediaLibrary::search(const QString& query) const
{
    if (query.trimmed().isEmpty()) return m_files;
    QVector<MediaFile> out;
    const QString needle = query.trimmed().toLower();
    for (const auto& file : m_files) {
        if (file.name.toLower().contains(needle) || file.path.toLower().contains(needle))
            out << file;
    }
    return out;
}

} // namespace ps::media
