#pragma once

#include "VideoEngine.h"

#include <QMediaPlayer>
#include <QAudioOutput>

namespace ps::media {

class MultimediaVideoEngine : public VideoEngine
{
    Q_OBJECT
public:
    explicit MultimediaVideoEngine(QObject* parent = nullptr);

    bool load(const QUrl& url) override;
    void play() override;
    void pause() override;
    void stop() override;
    void seek(qint64 milliseconds) override;
    void setLoop(bool loop) override;
    void setMuted(bool muted) override;
    void setVolume(double volume) override;

    QVideoSink* videoSink() const override { return m_sink; }
    bool hardwareAccelerated() const override { return false; }
    QString backendName() const override { return u"Qt Multimedia"; }

    qint64 position() const override { return m_player.position(); }
    qint64 duration() const override { return m_player.duration(); }
    PlaybackState state() const override;

private:
    QMediaPlayer m_player;
    QAudioOutput m_audio;
    QVideoSink* m_sink = nullptr;
    bool m_loop = false;
};

} // namespace ps::media
