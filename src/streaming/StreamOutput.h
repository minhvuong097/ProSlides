#pragma once

#include <QObject>
#include <QImage>
#include <QSize>

namespace ps::streaming {

// Live output to RTMP (e.g. rtmp://localhost/live) or HLS playlist/file.
// FFmpeg-backed; no-op stub without FFmpeg.
class StreamOutput : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QString target READ target NOTIFY activeChanged)

public:
    explicit StreamOutput(QObject* parent = nullptr);
    ~StreamOutput() override;

    bool start(const QString& url, const QSize& size, double fps = 30.0,
               bool hls = false);
    void pushFrame(const QImage& frame);
    void stop();
    bool active() const;
    QString target() const { return m_target; }

signals:
    void activeChanged(bool active);
    void errorOccurred(const QString& error);

private:
    class Private;
    Private* d = nullptr;
    QString m_target;
};

} // namespace ps::streaming
