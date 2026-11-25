#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QTimer>
#include <QElapsedTimer>

namespace ps::timeline {

// Playback engine for FreeShow-style timelines.
// Time base: action.time is milliseconds, action.duration is seconds.
class TimelineEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool playing READ playing NOTIFY stateChanged)
    Q_PROPERTY(bool paused READ paused NOTIFY stateChanged)
    Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
    Q_PROPERTY(qint64 duration READ duration NOTIFY actionsChanged)
    Q_PROPERTY(int actionCount READ actionCount NOTIFY actionsChanged)
    Q_PROPERTY(QVariantList actions READ actions NOTIFY actionsChanged)
    Q_PROPERTY(bool loop READ loop WRITE setLoop NOTIFY loopChanged)

public:
    explicit TimelineEngine(QObject* parent = nullptr);

    bool playing() const { return m_playing; }
    bool paused() const { return m_paused; }
    qint64 position() const { return m_position; }
    qint64 duration() const;
    int actionCount() const { return int(m_actions.size()); }
    QVariantList actions() const;
    bool loop() const { return m_loop; }

    Timeline timeline() const;
    void setTimeline(const Timeline& timeline);
    void setTimelineJson(const QJsonObject& json);

    void setLoop(bool loop);

public slots:
    void start();
    void pause();
    void stop();
    void seek(qint64 milliseconds);

    Q_INVOKABLE void addAction(const QString& name, const QString& type,
                               qint64 timeMs, double durationSeconds);
    Q_INVOKABLE bool removeAction(const QString& id);
    Q_INVOKABLE void updateActionTime(const QString& id, qint64 timeMs);
    Q_INVOKABLE void updateActionDuration(const QString& id, double seconds);
    Q_INVOKABLE void clear();

signals:
    void stateChanged();
    void positionChanged(qint64 ms);
    void actionsChanged();
    void loopChanged(bool loop);
    void actionTriggered(const TimelineAction& action);
    void finished();

private slots:
    void tick();

private:
    void rebuildActions();
    void triggerDue(qint64 from, qint64 to);
    int findAction(const QString& id) const;

    std::vector<TimelineAction> m_actions;
    QVariantList m_actionsModel;
    QTimer m_timer;
    QElapsedTimer m_clock;
    qint64 m_position = 0;
    qint64 m_offset = 0;
    bool m_playing = false;
    bool m_paused = false;
    bool m_loop = false;
    size_t m_nextIndex = 0;
};

} // namespace ps::timeline
