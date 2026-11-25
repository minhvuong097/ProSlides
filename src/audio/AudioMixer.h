#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QVector>

namespace ps::audio {

struct MixerChannel {
    Id id;
    QString name;
    QString path;
    QMediaPlayer* player = nullptr;
    QAudioOutput* output = nullptr;
    double volume = 0.8;
    bool muted = false;
    bool solo = false;
    bool loop = false;
};

// Multi-channel audio bed with mute/solo/volume, similar to FreeShow's
// audio routing matrix (without OS-level device routing).
class AudioMixer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList channels READ channels NOTIFY channelsChanged)

public:
    explicit AudioMixer(QObject* parent = nullptr);
    ~AudioMixer() override;

    QVariantList channels() const { return m_model; }

    Q_INVOKABLE QString addChannel(const QString& name, const QString& path);
    Q_INVOKABLE bool removeChannel(const QString& id);
    Q_INVOKABLE bool playChannel(const QString& id);
    Q_INVOKABLE bool stopChannel(const QString& id);
    Q_INVOKABLE void playAll();
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE bool setChannelVolume(const QString& id, double volume);
    Q_INVOKABLE bool toggleMute(const QString& id);
    Q_INVOKABLE bool toggleSolo(const QString& id);
    Q_INVOKABLE void clear();

signals:
    void channelsChanged();
    void errorOccurred(const QString& error);

private slots:
    void onStatus(QMediaPlayer::MediaStatus status);

private:
    void rebuild();
    void applyRouting();
    int find(const QString& id) const;

    QVector<MixerChannel> m_channels;
    QVariantList m_model;
};

} // namespace ps::audio
