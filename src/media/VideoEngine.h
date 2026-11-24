#pragma once

#include <QObject>
#include <QVideoFrame>
#include <QVideoSink>
#include <QUrl>

namespace ps::media {

enum class PlaybackState { Stopped, Playing, Paused, Ended };

QString playbackStateName(PlaybackState state);

// Common interface for foreground/background video playback.
// Two backends are provided: FFmpeg (hardware D3D11VA/DXVA2 on Windows) and
// Qt Multimedia (fallback for systems without FFmpeg dev headers).
class VideoEngine : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual bool load(const QUrl& url) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;
    virtual void seek(qint64 milliseconds) = 0;
    virtual void setLoop(bool loop) = 0;
    virtual void setMuted(bool muted) = 0;
    virtual void setVolume(double volume) = 0; // 0.0 .. 1.0

    virtual QVideoSink* videoSink() const = 0;
    virtual bool hardwareAccelerated() const = 0;
    virtual QString backendName() const = 0;

    virtual qint64 position() const = 0;
    virtual qint64 duration() const = 0;
    virtual PlaybackState state() const = 0;

signals:
    void positionChanged(qint64 ms);
    void durationChanged(qint64 ms);
    void stateChanged(PlaybackState state);
    void errorOccurred(const QString& error);
    void finished();
};

} // namespace ps::media
