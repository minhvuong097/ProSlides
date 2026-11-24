#pragma once

#include "Types.h"

#include <QObject>
#include <QVariantList>
#include <QMediaPlayer>
#include <QAudioOutput>

namespace ps::audio {

struct AudioTrack {
    Id id;
    QString path;
    QString name;
    qint64 duration = 0;
};

// Backing-track / worship music player with an in-memory playlist.
class AudioPlayer : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList tracks READ tracks NOTIFY tracksChanged)
    Q_PROPERTY(QString currentName READ currentName NOTIFY trackChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY trackChanged)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY volumeChanged)
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged)

public:
    explicit AudioPlayer(QObject* parent = nullptr);

    QVariantList tracks() const { return m_model; }
    QString currentName() const;
    qint64 position() const { return m_player.position(); }
    qint64 duration() const { return m_player.duration(); }
    bool playing() const { return m_playing; }
    double volume() const { return m_audio.volume(); }
    bool muted() const { return m_audio.isMuted(); }
    bool loop() const { return m_loop; }

    void setVolume(double volume);
    void setMuted(bool muted);
    void setLoop(bool loop);

    Q_INVOKABLE QString addTrack(const QString& path, const QString& name = {});
    Q_INVOKABLE bool removeTrack(const QString& id);
    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void next();
    Q_INVOKABLE void previous();
    Q_INVOKABLE void seek(qint64 milliseconds);
    Q_INVOKABLE bool selectTrack(const QString& id);
    Q_INVOKABLE void clear();

signals:
    void tracksChanged();
    void trackChanged();
    void positionChanged(qint64 ms);
    void playingChanged(bool playing);
    void volumeChanged();
    void loopChanged(bool loop);
    void errorOccurred(const QString& error);

private slots:
    void onStatus(QMediaPlayer::MediaStatus status);

private:
    void rebuild();
    void loadCurrent();
    int find(const QString& id) const;

    QMediaPlayer m_player;
    QAudioOutput m_audio;
    QVector<AudioTrack> m_tracks;
    QVariantList m_model;
    int m_index = -1;
    bool m_playing = false;
    bool m_loop = false;
};

} // namespace ps::audio
