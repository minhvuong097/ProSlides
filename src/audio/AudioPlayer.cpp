#include "AudioPlayer.h"

#include <QUrl>
#include <QFileInfo>
#include <QVariantMap>

namespace ps::audio {

AudioPlayer::AudioPlayer(QObject* parent)
    : QObject(parent)
{
    m_player.setAudioOutput(&m_audio);
    m_audio.setVolume(0.8);

    connect(&m_player, &QMediaPlayer::positionChanged,
            this, &AudioPlayer::positionChanged);
    connect(&m_player, &QMediaPlayer::mediaStatusChanged,
            this, &AudioPlayer::onStatus);
    connect(&m_player, &QMediaPlayer::errorOccurred, this,
            [this](QMediaPlayer::Error, const QString& message) {
                emit errorOccurred(message);
            });
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, [this]() {
        const bool nowPlaying = m_player.playbackState() == QMediaPlayer::PlayingState;
        if (nowPlaying != m_playing) {
            m_playing = nowPlaying;
            emit playingChanged(m_playing);
        }
    });
}

int AudioPlayer::find(const QString& id) const
{
    for (int i = 0; i < m_tracks.size(); ++i) {
        if (m_tracks[size_t(i)].id == id) return i;
    }
    return -1;
}

QString AudioPlayer::currentName() const
{
    if (m_index < 0 || m_index >= m_tracks.size()) return {};
    return m_tracks[size_t(m_index)].name;
}

void AudioPlayer::setVolume(double volume)
{
    m_audio.setVolume(qBound(0.0, volume, 1.0));
    emit volumeChanged();
}

void AudioPlayer::setMuted(bool muted)
{
    m_audio.setMuted(muted);
    emit volumeChanged();
}

void AudioPlayer::setLoop(bool loop)
{
    if (m_loop == loop) return;
    m_loop = loop;
    emit loopChanged(loop);
}

void AudioPlayer::rebuild()
{
    m_model.clear();
    for (int i = 0; i < m_tracks.size(); ++i) {
        const auto& track = m_tracks[size_t(i)];
        QVariantMap m;
        m.insert(QStringLiteral(u"id"), track.id);
        m.insert(QStringLiteral(u"name"), track.name);
        m.insert(QStringLiteral(u"path"), track.path);
        m.insert(QStringLiteral(u"duration"), track.duration);
        m.insert(QStringLiteral(u"active"), i == m_index);
        m_model.append(m);
    }
    emit tracksChanged();
}

QString AudioPlayer::addTrack(const QString& path, const QString& name)
{
    AudioTrack track;
    track.id = makeId();
    track.path = path;
    track.name = name.isEmpty() ? QFileInfo(path).completeBaseName() : name;
    m_tracks.append(track);
    if (m_index < 0) m_index = 0;
    rebuild();
    if (m_index == 0) loadCurrent();
    return track.id;
}

bool AudioPlayer::removeTrack(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    const bool wasCurrent = i == m_index;
    m_tracks.removeAt(i);
    if (m_tracks.isEmpty()) {
        stop();
        m_index = -1;
    } else if (i < m_index) {
        --m_index;
    } else if (i == m_index) {
        m_index = qMin(m_index, int(m_tracks.size()) - 1);
    }
    rebuild();
    if (wasCurrent && m_index >= 0) loadCurrent();
    return true;
}

void AudioPlayer::loadCurrent()
{
    if (m_index < 0 || m_index >= m_tracks.size()) return;
    m_player.setSource(QUrl::fromLocalFile(m_tracks[size_t(m_index)].path));
    emit trackChanged();
}

void AudioPlayer::play()
{
    if (m_tracks.isEmpty()) return;
    if (m_player.source().isEmpty()) loadCurrent();
    m_player.play();
}

void AudioPlayer::pause() { m_player.pause(); }
void AudioPlayer::stop() { m_player.stop(); }

void AudioPlayer::next()
{
    if (m_tracks.isEmpty()) return;
    m_index = (m_index + 1) % int(m_tracks.size());
    loadCurrent();
    rebuild();
    m_player.play();
}

void AudioPlayer::previous()
{
    if (m_tracks.isEmpty()) return;
    m_index = (m_index - 1 + int(m_tracks.size())) % int(m_tracks.size());
    loadCurrent();
    rebuild();
    m_player.play();
}

void AudioPlayer::seek(qint64 milliseconds) { m_player.setPosition(milliseconds); }

bool AudioPlayer::selectTrack(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    m_index = i;
    loadCurrent();
    rebuild();
    return true;
}

void AudioPlayer::clear()
{
    stop();
    m_tracks.clear();
    m_index = -1;
    rebuild();
    emit trackChanged();
}

void AudioPlayer::onStatus(QMediaPlayer::MediaStatus status)
{
    if (status != QMediaPlayer::EndOfMedia) return;
    if (m_loop) {
        m_player.setPosition(0);
        m_player.play();
    } else if (m_index >= 0 && m_index + 1 < int(m_tracks.size())) {
        next();
    } else {
        stop();
    }
}

} // namespace ps::audio
