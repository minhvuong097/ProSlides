#include "MultimediaVideoEngine.h"

namespace ps::media {

MultimediaVideoEngine::MultimediaVideoEngine(QObject* parent)
    : VideoEngine(parent)
    , m_sink(new QVideoSink(this))
{
    m_player.setAudioOutput(&m_audio);
    m_player.setVideoSink(m_sink);

    connect(&m_player, &QMediaPlayer::positionChanged,
            this, &VideoEngine::positionChanged);
    connect(&m_player, &QMediaPlayer::durationChanged,
            this, &VideoEngine::durationChanged);
    connect(&m_player, &QMediaPlayer::errorOccurred,
            this, [this](QMediaPlayer::Error, const QString& message) {
                emit errorOccurred(message);
            });
    connect(&m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::EndOfMedia) {
            if (m_loop) {
                m_player.setPosition(0);
                m_player.play();
            } else {
                emit finished();
            }
        }
    });
    connect(&m_player, &QMediaPlayer::playbackStateChanged, this, [this]() {
        emit stateChanged(state());
    });
}

bool MultimediaVideoEngine::load(const QUrl& url)
{
    m_player.setSource(url);
    return true;
}

void MultimediaVideoEngine::play() { m_player.play(); }
void MultimediaVideoEngine::pause() { m_player.pause(); }
void MultimediaVideoEngine::stop() { m_player.stop(); }
void MultimediaVideoEngine::seek(qint64 milliseconds) { m_player.setPosition(milliseconds); }
void MultimediaVideoEngine::setLoop(bool loop) { m_loop = loop; }
void MultimediaVideoEngine::setMuted(bool muted) { m_audio.setMuted(muted); }
void MultimediaVideoEngine::setVolume(double volume) { m_audio.setVolume(volume); }

PlaybackState MultimediaVideoEngine::state() const
{
    switch (m_player.playbackState()) {
    case QMediaPlayer::PlayingState: return PlaybackState::Playing;
    case QMediaPlayer::PausedState: return PlaybackState::Paused;
    case QMediaPlayer::StoppedState: return PlaybackState::Stopped;
    }
    return PlaybackState::Stopped;
}

} // namespace ps::media
