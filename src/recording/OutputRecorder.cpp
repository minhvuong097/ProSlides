#include "OutputRecorder.h"

#if defined(PS_HAS_FFMPEG)
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

namespace ps::recording {

#if defined(PS_HAS_FFMPEG)

class OutputRecorder::Private
{
public:
    AVFormatContext* format = nullptr;
    AVCodecContext* codec = nullptr;
    AVStream* stream = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;
    SwsContext* sws = nullptr;
    int width = 0;
    int height = 0;
    double fps = 30.0;
    qint64 pts = 0;
    bool active = false;
    bool headerWritten = false;
};

OutputRecorder::OutputRecorder(QObject* parent)
    : QObject(parent), d(new Private)
{
}

OutputRecorder::~OutputRecorder()
{
    stop();
    delete d;
}

bool OutputRecorder::start(const QString& path, const QSize& size, double fps)
{
    if (d->active) return false;
    d->width = size.width() & ~1;
    d->height = size.height() & ~1;
    d->fps = fps > 0 ? fps : 30.0;
    if (d->width < 2 || d->height < 2) return false;

    if (avformat_alloc_output_context2(&d->format, nullptr, "mp4", qPrintable(path)) < 0 ||
        !d->format) {
        emit errorOccurred(QStringLiteral(u"Cannot allocate output context"));
        return false;
    }

    const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!encoder) encoder = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if (!encoder) {
        emit errorOccurred(QStringLiteral(u"No MP4 encoder available"));
        return false;
    }

    d->stream = avformat_new_stream(d->format, nullptr);
    if (!d->stream) {
        emit errorOccurred(QStringLiteral(u"Cannot create stream"));
        return false;
    }
    d->codec = avcodec_alloc_context3(encoder);
    d->codec->codec_id = encoder->id;
    d->codec->codec_type = AVMEDIA_TYPE_VIDEO;
    d->codec->width = d->width;
    d->codec->height = d->height;
    d->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    d->codec->time_base = { 1, 90000 };
    d->codec->framerate = { int(d->fps * 1000), 1000 };
    if (d->format->oformat->flags & AVFMT_GLOBALHEADER)
        d->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (avcodec_open2(d->codec, encoder, nullptr) < 0) {
        emit errorOccurred(QStringLiteral(u"Cannot open encoder"));
        return false;
    }
    avcodec_parameters_from_context(d->stream->codecpar, d->codec);
    d->stream->time_base = d->codec->time_base;

    if (!(d->format->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&d->format->pb, qPrintable(path), AVIO_FLAG_WRITE) < 0) {
            emit errorOccurred(QStringLiteral(u"Cannot open output file"));
            return false;
        }
    }
    if (avformat_write_header(d->format, nullptr) < 0) {
        emit errorOccurred(QStringLiteral(u"Cannot write MP4 header"));
        return false;
    }
    d->headerWritten = true;

    d->frame = av_frame_alloc();
    d->frame->format = AV_PIX_FMT_YUV420P;
    d->frame->width = d->width;
    d->frame->height = d->height;
    av_frame_get_buffer(d->frame, 32);
    d->packet = av_packet_alloc();
    d->sws = sws_getContext(d->width, d->height, AV_PIX_FMT_ARGB32,
                            d->width, d->height, AV_PIX_FMT_YUV420P,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!d->sws) {
        emit errorOccurred(QStringLiteral(u"Cannot create scaler"));
        return false;
    }

    d->pts = 0;
    d->active = true;
    m_path = path;
    emit recordingChanged(true);
    return true;
}

void OutputRecorder::pushFrame(const QImage& frame)
{
    if (!d->active || frame.isNull()) return;

    const QImage rgb = frame.convertToFormat(QImage::Format_ARGB32)
                           .scaled(d->width, d->height, Qt::KeepAspectRatio,
                                   Qt::FastTransformation);
    const uint8_t* src[] = { reinterpret_cast<const uint8_t*>(rgb.constBits()) };
    const int srcStride[] = { rgb.bytesPerLine() };
    av_frame_make_writable(d->frame);
    sws_scale(d->sws, src, srcStride, 0, d->height, d->frame->data, d->frame->linesize);


    d->frame->pts = d->pts;
    d->pts += qint64(90000.0 / d->fps);

    if (avcodec_send_frame(d->codec, d->frame) < 0) return;
    while (avcodec_receive_packet(d->codec, d->packet) == 0) {
        av_packet_rescale_ts(d->packet, d->codec->time_base, d->stream->time_base);
        d->packet->stream_index = d->stream->index;
        av_interleaved_write_frame(d->format, d->packet);
        av_packet_unref(d->packet);
    }
}

void OutputRecorder::stop()
{
    if (!d->active) return;
    d->active = false;

    avcodec_send_frame(d->codec, nullptr);
    while (avcodec_receive_packet(d->codec, d->packet) == 0) {
        av_packet_rescale_ts(d->packet, d->codec->time_base, d->stream->time_base);
        d->packet->stream_index = d->stream->index;
        av_interleaved_write_frame(d->format, d->packet);
        av_packet_unref(d->packet);
    }
    if (d->headerWritten) av_write_trailer(d->format);
    if (d->format && !(d->format->oformat->flags & AVFMT_NOFILE)) avio_closep(&d->format->pb);
    if (d->sws) sws_freeContext(d->sws);
    if (d->packet) av_packet_free(&d->packet);
    if (d->frame) av_frame_free(&d->frame);
    if (d->codec) avcodec_free_context(&d->codec);
    if (d->format) avformat_free_context(d->format);
    d->format = nullptr;
    d->codec = nullptr;
    d->stream = nullptr;
    d->frame = nullptr;
    d->packet = nullptr;
    d->sws = nullptr;
    d->headerWritten = false;
    m_path.clear();
    emit recordingChanged(false);
}

bool OutputRecorder::recording() const
{
    return d->active;
}

#else // no FFmpeg

class OutputRecorder::Private {};

OutputRecorder::OutputRecorder(QObject* parent) : QObject(parent), d(new Private) {}
OutputRecorder::~OutputRecorder() { delete d; }
bool OutputRecorder::start(const QString&, const QSize&, double) { emit errorOccurred(QStringLiteral(u"FFmpeg support not compiled in")); return false; }
void OutputRecorder::pushFrame(const QImage&) {}
void OutputRecorder::stop() {}
bool OutputRecorder::recording() const { return false; }

#endif

} // namespace ps::recording
