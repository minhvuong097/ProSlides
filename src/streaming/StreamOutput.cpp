#include "StreamOutput.h"

#if defined(PS_HAS_FFMPEG)
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

namespace ps::streaming {

#if defined(PS_HAS_FFMPEG)

class StreamOutput::Private
{
public:
    AVFormatContext* format = nullptr;
    AVCodecContext* codec = nullptr;
    AVStream* stream = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* packet = nullptr;
    SwsContext* sws = nullptr;
    int width = 0, height = 0;
    double fps = 30.0;
    qint64 pts = 0;
    bool active = false;
};

StreamOutput::StreamOutput(QObject* parent)
    : QObject(parent), d(new Private)
{
}

StreamOutput::~StreamOutput()
{
    stop();
    delete d;
}

bool StreamOutput::start(const QString& url, const QSize& size, double fps, bool hls)
{
    if (d->active) return false;
    d->width = size.width() & ~1;
    d->height = size.height() & ~1;
    d->fps = fps > 0 ? fps : 30.0;
    if (d->width < 2 || d->height < 2 || url.isEmpty()) return false;

    const char* formatName = hls ? "hls" : "flv";
    if (avformat_alloc_output_context2(&d->format, nullptr, formatName, qPrintable(url)) < 0 ||
        !d->format) {
        emit errorOccurred(QStringLiteral(u"Cannot allocate stream context"));
        return false;
    }

    const AVCodec* encoder = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!encoder) encoder = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if (!encoder) {
        emit errorOccurred(QStringLiteral(u"No encoder available"));
        return false;
    }
    d->stream = avformat_new_stream(d->format, nullptr);
    if (!d->stream) return false;
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
        if (avio_open(&d->format->pb, qPrintable(url), AVIO_FLAG_WRITE) < 0) {
            emit errorOccurred(QStringLiteral(u"Cannot open output stream"));
            return false;
        }
    }
    if (avformat_write_header(d->format, nullptr) < 0) {
        emit errorOccurred(QStringLiteral(u"Cannot write stream header"));
        return false;
    }

    d->frame = av_frame_alloc();
    d->frame->format = AV_PIX_FMT_YUV420P;
    d->frame->width = d->width;
    d->frame->height = d->height;
    av_frame_get_buffer(d->frame, 32);
    d->packet = av_packet_alloc();
    d->sws = sws_getContext(d->width, d->height, AV_PIX_FMT_ARGB32,
                            d->width, d->height, AV_PIX_FMT_YUV420P,
                            SWS_BILINEAR, nullptr, nullptr, nullptr);
    d->pts = 0;
    d->active = true;
    m_target = url;
    emit activeChanged(true);
    return true;
}

void StreamOutput::pushFrame(const QImage& frame)
{
    if (!d->active || frame.isNull() || !d->sws) return;
    const QImage rgb = frame.convertToFormat(QImage::Format_ARGB32)
                           .scaled(d->width, d->height, Qt::KeepAspectRatio, Qt::FastTransformation);
    const uint8_t* src[] = { reinterpret_cast<const uint8_t*>(rgb.constBits()) };
    const int srcStride[] = { rgb.bytesPerLine() };
    av_frame_make_writable(d->frame);
    sws_scale(d->sws, src, srcStride, 0, d->height, d->frame->data, d->frame->linesize);
    d->frame->pts = d->pts++;
    if (avcodec_send_frame(d->codec, d->frame) < 0) return;
    while (avcodec_receive_packet(d->codec, d->packet) == 0) {
        av_packet_rescale_ts(d->packet, d->codec->time_base, d->stream->time_base);
        d->packet->stream_index = d->stream->index;
        av_interleaved_write_frame(d->format, d->packet);
        av_packet_unref(d->packet);
    }
}

void StreamOutput::stop()
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
    av_write_trailer(d->format);
    if (!(d->format->oformat->flags & AVFMT_NOFILE)) avio_closep(&d->format->pb);
    if (d->sws) sws_freeContext(d->sws);
    if (d->packet) av_packet_free(&d->packet);
    if (d->frame) av_frame_free(&d->frame);
    if (d->codec) avcodec_free_context(&d->codec);
    if (d->format) avformat_free_context(d->format);
    d->format = nullptr; d->codec = nullptr; d->stream = nullptr;
    d->packet = nullptr; d->frame = nullptr; d->sws = nullptr;
    m_target.clear();
    emit activeChanged(false);
}

bool StreamOutput::active() const
{
    return d->active;
}

#else

class StreamOutput::Private {};
StreamOutput::StreamOutput(QObject* parent) : QObject(parent), d(new Private) {}
StreamOutput::~StreamOutput() { delete d; }
bool StreamOutput::start(const QString&, const QSize&, double, bool) { emit errorOccurred(QStringLiteral(u"FFmpeg support not compiled in")); return false; }
void StreamOutput::pushFrame(const QImage&) {}
void StreamOutput::stop() {}
bool StreamOutput::active() const { return false; }

#endif

} // namespace ps::streaming
