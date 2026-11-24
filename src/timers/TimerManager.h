#pragma once

#include "core/Types.h"

#include <QObject>
#include <QVariantList>
#include <QTimer>

namespace ps::timers {

enum class TimerKind { Countdown, Clock, Stopwatch };

struct Timer {
    Id id;
    QString name;
    TimerKind kind = TimerKind::Countdown;
    qint64 durationMs = 0;      // countdown length
    qint64 startMs = 0;         // epoch ms of last start
    qint64 accumulatedMs = 0;   // time spent running before pause
    bool running = false;
    int warnSeconds = 0;
    QString warnColor = QStringLiteral(u"#ff6b6b");
};

// Service timers: countdown, clock and stopwatch. Ticks at 250 ms and exposes
// a QVariantList model for QML / the phone remote.
class TimerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList timers READ timers NOTIFY timersChanged)
    Q_PROPERTY(int activeCount READ activeCount NOTIFY timersChanged)

public:
    explicit TimerManager(QObject* parent = nullptr);

    QVariantList timers() const { return m_model; }
    int activeCount() const;
    Q_INVOKABLE QString startTimer(const QString& name, double seconds, int warnSeconds = 0);
    Q_INVOKABLE bool pauseTimer(const QString& id);
    Q_INVOKABLE bool resumeTimer(const QString& id);
    Q_INVOKABLE bool stopTimer(const QString& id);
    Q_INVOKABLE bool toggleTimer(const QString& id);
    Q_INVOKABLE void pauseAll();
    Q_INVOKABLE void stopAll();
    Q_INVOKABLE void clearAll();

signals:
    void timersChanged();
    void timerFinished(const QString& id, const QString& name);

private slots:
    void tick();

private:
    void rebuild();
    qint64 elapsed(const Timer& timer) const;
    int find(const QString& id) const;

    QVector<Timer> m_timers;
    QVariantList m_model;
    QTimer m_tick;
};

} // namespace ps::timers
