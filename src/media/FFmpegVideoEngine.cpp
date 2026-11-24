#include "FFmpegVideoEngine.h"

#include <QTimer>
#include <QElapsedTimer>
#include <QImage>
#include <QFileInfo>
#include <QMetaObject>
#include <atomic>

#if defined(PS_HAS_FFMPEG)
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/hwcontext.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}
#endif

namespace ps::media {

#if defined(PS_HAS_FFMPEG)

namespace {
thread_local AVPixelFormat t_hwPixelFormat = AV_PIX_FMT_NONE;

AVPixelFormat selectFormat(AVCodecContext*, const AVPixelFormat* formats)
{
    for (const AVPixelFormat* f = formats; *f != AV_PIX_FMT_NONE; ++f) {
        if (*f == t_hwPixelFormat) return *f;
    }
    return AV_PIX_FMT_NONE;
}

AVHWDeviceType preferredDeviceType()
{
#if defined(Q_OS_WIN)
    // D3D11VA gives the lowest CPU overhead for 4K worship loops.
    return AV_HWDEVICE_TYPE_D3D11VA;
#elif defined(Q_OS_MACOS)
    return AV_HWDEVICE_TYPE_VIDEOTOOLBOX;
#else
    return AV_HWDEVICE_TYPE_VAAPI;
#endif
}
} // namespace

class FFmpegDecodeWorker : public QObject
{
    Q_OBJECT
public:
    explicit FFmpegDecodeWorker(QObject* parent = nullptr)
        : QObject(parent)
    {
        m_timer = new QTimer(this);
        m_timer->setInterval(16);
        connect(m_timer, &QTimer::timeout, this, &FFmpegDecodeWorker::decodeStep);
    }

    ~FFmpegDecodeWorker() override { release(); }

public slots:
    bool load(const QString& path)
    {
        release();
        m_path = path;
        int ret = avformat_open_input(&m_format, qPrintable(path), nullptr, nullptr);
        if (ret < 0) { emit error(QStringLiteral("avformat_open_input: %1").arg(ret)); return false; }

        if (avformat_find_stream_info(m_format, nullptr) < 0) {
            emit error(QStringLiteral("avformat_find_stream_info failed"));
            return false;
        }
        const int streamIndex = av_find_best_stream(m_format, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
        if (streamIndex < 0) { emit error(QStringLiteral("No video stream")); return false; }
        m_streamIndex = streamIndex;
        AVStream* stream = m_format->streams[streamIndex];

        const AVCodec* codec = avcodec_find_decoder(stream->codecpar->codec_id);
        if (!codec) { emit error(QStringLiteral("No decoder for codec")); return false; }
        m_codec = avcodec_alloc_context3(codec);
        avcodec_parameters_to_context(m_codec, stream->codecpar);

        // --- hardware decoding -------------------------------------------
        m_hardware = false;
        t_hwPixelFormat = AV_PIX_FMT_NONE;
        const AVHWDeviceType hwType = preferredDeviceType();
        if (av_hwdevice_ctx_create(&m_hwDevice, hwType, nullptr, nullptr, 0) >= 0) {
            AVHWFramesConstraints* constraints = av_hwdevice_get_hwframe_constraints(m_hwDevice, nullptr);
            if (constraints && constraints->valid_hw_formats &&
                constraints->valid_hw_formats[0] != AV_PIX_FMT_NONE) {
                t_hwPixelFormat = constraints->valid_hw_formats[0];
                m_codec->get_format = selectFormat;
                m_codec->hw_device_ctx = av_buffer_ref(m_hwDevice);
                m_hardware = true;
            }
            if (constraints) av_hwframe_constraints_free(constraints);
        }
        // Fall back to software decoding if hardware init failed.
        if (!m_hardware) t_hwPixelFormat = AV_PIX_FMT_NONE;

        if (avcodec_open2(m_codec, codec, nullptr) < 0) {
            emit error(QStringLiteral("avcodec_open2 failed"));
            return false;
        }

        m_packet = av_packet_alloc();
        m_frame = av_frame_alloc();
        m_swFrame = av_frame_alloc();
        m_timeBase = stream->time_base;
        const qint64 duration = stream->duration != AV_NOPTS_VALUE
            ? av_rescale_q(stream->duration, m_timeBase, AV_TIME_BASE_Q) / 1000
            : (m_format->duration != AV_NOPTS_VALUE ? m_format->duration / 1000 : 0);
        emit durationChanged(duration);
        emit hardwareChanged(m_hardware);
        return true;
    }

    void play()
    {
        if (!m_codec) return;
        m_playing = true;
        m_clock.start();
        if (!m_timer->isActive()) m_timer->start();
        emit stateChanged(int(PlaybackState::Playing));
    }
    void pause()
    {
        m_playing = false;
        emit stateChanged(int(PlaybackState::Paused));
    }
    void stop()
    {
        m_playing = false;
        m_timer->stop();
        emit stateChanged(int(PlaybackState::Stopped));
    }
    void seek(qint64 ms)
    {
        if (!m_format) return;
        const int64_t ts = av_rescale_q(ms * 1000, AV_TIME_BASE_Q, m_timeBase);
        av_seek_frame(m_format, m_streamIndex, ts, AVSEEK_FLAG_BACKWARD);
        avcodec_flush_buffers(m_codec);
        m_clock.start();
        m_basePts = av_rescale_q(ts, m_timeBase, AV_TIME_BASE_Q) / 1000;
    }
    void setLoop(bool loop) { m_loop = loop; }
    void setMuted(bool) { /* audio mixing is owned by the presentation layer */ }
    void setVolume(double) { /* audio mixing is owned by the presentation layer */ }

signals:
    void frameReady(const QVideoFrame& frame);
    void positionChanged(qint64 ms);
    void durationChanged(qint64 ms);
    void stateChanged(int state);
    void hardwareChanged(bool hw);
    void error(const QString& message);
    void finished();

private slots:
    void decodeStep()
    {
        if (!m_playing || !m_codec) return;
        const qint64 elapsed = m_clock.elapsed() - m_seekOffset;

        for (int budget = 0; budget < 4; ++budget) {
            int ret = av_read_frame(m_format, m_packet);
            if (ret < 0) {
                if (m_loop) {
                    seek(0);
                    return;
                }
                m_timer->stop();
                m_playing = false;
                emit stateChanged(int(PlaybackState::Ended));
                emit finished();
                return;
            }
            if (m_packet->stream_index != m_streamIndex) { av_packet_unref(m_packet); continue; }

            ret = avcodec_send_packet(m_codec, m_packet);
            av_packet_unref(m_packet);
            if (ret < 0) return;

            ret = avcodec_receive_frame(m_codec, m_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return;
            if (ret < 0) return;

            const qint64 ptsMs = m_frame->best_effort_timestamp != AV_NOPTS_VALUE
                ? av_rescale_q(m_frame->best_effort_timestamp, m_timeBase, AV_TIME_BASE_Q) / 1000
                : 0;
            if (ptsMs > elapsed) {
                av_frame_unref(m_frame);
                return;
            }

            AVFrame* source = m_frame;
            if (m_frame->format == t_hwPixelFormat && t_hwPixelFormat != AV_PIX_FMT_NONE) {
                if (av_hwframe_transfer_data(m_swFrame, m_frame, 0) < 0) {
                    av_frame_unref(m_frame);
                    return;
                }
                source = m_swFrame;
            }

            QImage image = toImage(source);
            if (!image.isNull()) emit frameReady(QVideoFrame(image));
            emit positionChanged(ptsMs);
            av_frame_unref(m_frame);
            return;
        }
    }

private:
    QImage toImage(AVFrame* frame)
    {
        const int w = frame->width, h = frame->height;
        if (w <= 0 || h <= 0) return {};

        const AVPixelFormat fmt = static_cast<AVPixelFormat>(frame->format);
        if (!m_sws || m_swsW != w || m_swsH != h || m_swsFmt != fmt) {
            if (m_sws) sws_freeContext(m_sws);
            m_sws = sws_getContext(w, h, fmt, w, h, AV_PIX_FMT_ARGB32,
                                   SWS_BILINEAR, nullptr, nullptr, nullptr);
            m_swsW = w; m_swsH = h; m_swsFmt = fmt;
        }
        if (!m_sws) return {};

        QImage image(w, h, QImage::Format_ARGB32);
        uint8_t* dst[] = { image.bits() };
        const int dstStride[] = { image.bytesPerLine() };
        sws_scale(m_sws, frame->data, frame->linesize, 0, h, dst, dstStride);
        return image;
    }

    void release()
    {
        m_timer->stop();
        if (m_sws) { sws_freeContext(m_sws); m_sws = nullptr; }
        if (m_frame) { av_frame_free(&m_frame); }
        if (m_swFrame) { av_frame_free(&m_swFrame); }
        if (m_packet) { av_packet_free(&m_packet); }
        if (m_codec) { avcodec_free_context(&m_codec); }
        if (m_format) { avformat_close_input(&m_format); }
        if (m_hwDevice) { av_buffer_unref(&m_hwDevice); }
    }

    QString m_path;
    AVFormatContext* m_format = nullptr;
    AVCodecContext* m_codec = nullptr;
    AVBufferRef* m_hwDevice = nullptr;
    AVPacket* m_packet = nullptr;
    AVFrame* m_frame = nullptr;
    AVFrame* m_swFrame = nullptr;
    SwsContext* m_sws = nullptr;
    AVRational m_timeBase = { 1, 90000 };
    int m_swsW = 0, m_swsH = 0;
    AVPixelFormat m_swsFmt = AV_PIX_FMT_NONE;
    int m_streamIndex = -1;
    bool m_playing = false;
    bool m_loop = false;
    bool m_hardware = false;
    qint64 m_basePts = 0;
    qint64 m_seekOffset = 0;
    QTimer* m_timer = nullptr;
    QElapsedTimer m_clock;
};

// ---------------------------------------------------------------------------

FFmpegVideoEngine::FFmpegVideoEngine(QObject* parent)
    : VideoEngine(parent)
    , m_sink(new QVideoSink(this))
{
    m_worker = new FFmpegDecodeWorker();
    m_worker->moveToThread(&m_thread);

    connect(m_worker, &FFmpegDecodeWorker::frameReady,
            m_sink, &QVideoSink::setVideoFrame);
    connect(m_worker, &FFmpegDecodeWorker::positionChanged,
            this, &VideoEngine::positionChanged);
    connect(m_worker, &FFmpegDecodeWorker::durationChanged,
            this, &VideoEngine::durationChanged);
    connect(m_worker, &FFmpegDecodeWorker::error,
            this, &VideoEngine::errorOccurred);
    connect(m_worker, &FFmpegDecodeWorker::finished,
            this, &VideoEngine::finished);
    connect(m_worker, &FFmpegDecodeWorker::stateChanged, this, [this](int s) {
        emit stateChanged(static_cast<PlaybackState>(s));
    });
    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    m_thread.start();
}

FFmpegVideoEngine::~FFmpegVideoEngine()
{
    QMetaObject::invokeMethod(m_worker, "stop", Qt::BlockingQueuedConnection);
    m_thread.quit();
    m_thread.wait();
}

bool FFmpegVideoEngine::load(const QUrl& url)
{
    bool ok = false;
    QMetaObject::invokeMethod(m_worker, "load", Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(bool, ok), Q_ARG(QString, url.toLocalFile()));
    return ok;
}
void FFmpegVideoEngine::play()    { QMetaObject::invokeMethod(m_worker, "play"); }
void FFmpegVideoEngine::pause()   { QMetaObject::invokeMethod(m_worker, "pause"); }
void FFmpegVideoEngine::stop()    { QMetaObject::invokeMethod(m_worker, "stop"); }
void FFmpegVideoEngine::seek(qint64 ms) { QMetaObject::invokeMethod(m_worker, "seek", Q_ARG(qint64, ms)); }
void FFmpegVideoEngine::setLoop(bool loop) { QMetaObject::invokeMethod(m_worker, "setLoop", Q_ARG(bool, loop)); }
void FFmpegVideoEngine::setMuted(bool muted) { QMetaObject::invokeMethod(m_worker, "setMuted", Q_ARG(bool, muted)); }
void FFmpegVideoEngine::setVolume(double volume) { QMetaObject::invokeMethod(m_worker, "setVolume", Q_ARG(double, volume)); }

bool FFmpegVideoEngine::hardwareAccelerated() const { return true; }

#else // !PS_HAS_FFMPEG — stub backend

FFmpegVideoEngine::FFmpegVideoEngine(QObject* parent)
    : VideoEngine(parent), m_sink(new QVideoSink(this))
{
}
FFmpegVideoEngine::~FFmpegVideoEngine() = default;
bool FFmpegVideoEngine::load(const QUrl&) { emit errorOccurred(u"FFmpeg support not compiled in"); return false; }
void FFmpegVideoEngine::play() {}
void FFmpegVideoEngine::pause() {}
void FFmpegVideoEngine::stop() {}
void FFmpegVideoEngine::seek(qint64) {}
void FFmpegVideoEngine::setLoop(bool) {}
void FFmpegVideoEngine::setMuted(bool) {}
void FFmpegVideoEngine::setVolume(double) {}
bool FFmpegVideoEngine::hardwareAccelerated() const { return false; }
qint64 FFmpegVideoEngine::position() const { return 0; }
qint64 FFmpegVideoEngine::duration() const { return 0; }
PlaybackState FFmpegVideoEngine::state() const { return PlaybackState::Stopped; }

#endif

} // namespace ps::media

#if defined(PS_HAS_FFMPEG)
#include "FFmpegVideoEngine.moc"
#endif
