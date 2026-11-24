#pragma once

#include <QObject>
#include <QImage>
#include <QSize>

namespace ps::recording {

// Encodes frames pushed from the audience window into an MP4 file.
// FFmpeg-backed; no-op stub when built without FFmpeg.
class OutputRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(QString outputPath READ outputPath NOTIFY recordingChanged)

public:
    explicit OutputRecorder(QObject* parent = nullptr);
    ~OutputRecorder() override;

    bool start(const QString& path, const QSize& size, double fps = 30.0);
    void pushFrame(const QImage& frame);
    void stop();
    bool recording() const;
    QString outputPath() const { return m_path; }

signals:
    void recordingChanged(bool recording);
    void errorOccurred(const QString& error);

private:
    class Private;
    Private* d = nullptr;
    QString m_path;
};

} // namespace ps::recording
