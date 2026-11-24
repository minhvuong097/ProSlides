#pragma once

#include "VideoEngine.h"

#include <QThread>

namespace ps::media {

class FFmpegDecodeWorker;

// FFmpeg-based playback with hardware decoding (D3D11VA / DXVA2 on Windows,
// VideoToolbox on macOS, VAAPI on Linux) and a software fallback.
// Decoding runs on a dedicated thread; frames are delivered to QVideoSink.
class FFmpegVideoEngine : public VideoEngine
{
    Q_OBJECT
public:
    explicit FFmpegVideoEngine(QObject* parent = nullptr);
    ~FFmpegVideoEngine() override;

    bool load(const QUrl& url) override;
    void play() override;
    void pause() override;
    void stop() override;
    void seek(qint64 milliseconds) override;
    void setLoop(bool loop) override;
    void setMuted(bool muted) override;
    void setVolume(double volume) override;

    QVideoSink* videoSink() const override { return m_sink; }
    bool hardwareAccelerated() const override;
    QString backendName() const override { return u"FFmpeg (D3D11VA)"; }

    qint64 position() const override;
    qint64 duration() const override;
    PlaybackState state() const override;

private:
    QThread m_thread;
    FFmpegDecodeWorker* m_worker = nullptr;
    QVideoSink* m_sink = nullptr;
};

} // namespace ps::media
