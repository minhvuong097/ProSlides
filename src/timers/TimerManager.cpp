#include "TimerManager.h"

#include <QDateTime>
#include <QVariantMap>

namespace ps::timers {

TimerManager::TimerManager(QObject* parent)
    : QObject(parent)
{
    m_tick.setInterval(250);
    connect(&m_tick, &QTimer::timeout, this, &TimerManager::tick);
}

int TimerManager::find(const QString& id) const
{
    for (int i = 0; i < m_timers.size(); ++i) {
        if (m_timers[size_t(i)].id == id) return i;
    }
    return -1;
}

int TimerManager::activeCount() const
{
    int count = 0;
    for (const auto& timer : m_timers) if (timer.running) ++count;
    return count;
}

qint64 TimerManager::elapsed(const Timer& timer) const
{
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    return timer.accumulatedMs + (timer.running && timer.startMs > 0 ? now - timer.startMs : 0);
}

QString TimerManager::startTimer(const QString& name, double seconds, int warnSeconds)
{
    Timer timer;
    timer.id = makeId();
    timer.name = name.isEmpty() ? u"Timer" : name;
    timer.kind = TimerKind::Countdown;
    timer.durationMs = qint64(qMax(0.0, seconds) * 1000.0);
    timer.startMs = QDateTime::currentMSecsSinceEpoch();
    timer.running = true;
    timer.warnSeconds = warnSeconds;
    m_timers.append(timer);
    if (!m_tick.isActive()) m_tick.start();
    rebuild();
    return timer.id;
}

bool TimerManager::pauseTimer(const QString& id)
{
    const int i = find(id);
    if (i < 0 || !m_timers[size_t(i)].running) return false;
    auto& timer = m_timers[size_t(i)];
    timer.accumulatedMs = elapsed(timer);
    timer.running = false;
    timer.startMs = 0;
    rebuild();
    return true;
}

bool TimerManager::resumeTimer(const QString& id)
{
    const int i = find(id);
    if (i < 0 || m_timers[size_t(i)].running) return false;
    auto& timer = m_timers[size_t(i)];
    timer.startMs = QDateTime::currentMSecsSinceEpoch();
    timer.running = true;
    rebuild();
    return true;
}

bool TimerManager::stopTimer(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    m_timers.removeAt(i);
    rebuild();
    return true;
}

bool TimerManager::toggleTimer(const QString& id)
{
    const int i = find(id);
    if (i < 0) return false;
    return m_timers[size_t(i)].running ? pauseTimer(id) : resumeTimer(id);
}

void TimerManager::pauseAll()
{
    for (const auto& timer : m_timers) if (timer.running) pauseTimer(timer.id);
}

void TimerManager::stopAll()
{
    m_timers.clear();
    rebuild();
}

void TimerManager::clearAll()
{
    stopAll();
}

void TimerManager::tick()
{
    bool changed = false;
    for (const auto& timer : m_timers) {
        if (!timer.running || timer.kind != TimerKind::Countdown) continue;
        if (elapsed(timer) >= timer.durationMs) {
            const QString id = timer.id, name = timer.name;
            m_timers[find(id)].running = false;
            m_timers[find(id)].accumulatedMs = timer.durationMs;
            emit timerFinished(id, name);
            changed = true;
        }
    }
    rebuild();
    if (m_timers.isEmpty()) m_tick.stop();
    Q_UNUSED(changed);
}

void TimerManager::rebuild()
{
    m_model.clear();
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (const auto& timer : m_timers) {
        const qint64 used = elapsed(timer);
        const qint64 remaining = timer.kind == TimerKind::Countdown
            ? timer.durationMs - used
            : used;

        qint64 absMs = qAbs(remaining);
        const int hours = int(absMs / 3600000);
        const int minutes = int((absMs % 3600000) / 60000);
        const int seconds = int((absMs % 60000) / 1000);
        const QString text = (remaining < 0 ? u"-" : u"") +
            (hours > 0 ? QString::number(hours) + u":" + QString("%1").arg(minutes, 2, 10, QChar(u'0'))
                       : QString("%1").arg(minutes, 2, 10, QChar(u'0'))) +
            u":" + QString("%1").arg(seconds, 2, 10, QChar(u'0'));

        QVariantMap m;
        m.insert(u"id", timer.id);
        m.insert(u"name", timer.name);
        m.insert(u"running", timer.running);
        m.insert(u"remaining", remaining);
        m.insert(u"text", text);
        m.insert(u"warning", timer.warnSeconds > 0 && remaining <= timer.warnSeconds * 1000);
        m.insert(u"warnColor", timer.warnColor);
        Q_UNUSED(now);
        m_model.append(m);
    }
    emit timersChanged();
}

} // namespace ps::timers
