#include "TimelineEngine.h"

#include <QJsonObject>
#include <algorithm>

namespace ps::timeline {

TimelineEngine::TimelineEngine(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(16);
    connect(&m_timer, &QTimer::timeout, this, &TimelineEngine::tick);
}

QVariantList TimelineEngine::actions() const
{
    return m_actionsModel;
}

qint64 TimelineEngine::duration() const
{
    qint64 end = 0;
    for (const auto& action : m_actions)
        end = std::max(end, action.time + qint64(action.duration * 1000.0));
    return end;
}

Timeline TimelineEngine::timeline() const
{
    Timeline t;
    t.actions = m_actions;
    t.loop = m_loop;
    return t;
}

void TimelineEngine::setTimeline(const Timeline& timeline)
{
    stop();
    m_actions = timeline.actions;
    m_loop = timeline.loop;
    std::stable_sort(m_actions.begin(), m_actions.end(),
                     [](const auto& a, const auto& b) { return a.time < b.time; });
    rebuildActions();
    emit actionsChanged();
    emit loopChanged(m_loop);
}

void TimelineEngine::setTimelineJson(const QJsonObject& json)
{
    setTimeline(Timeline::fromJson(json));
}

void TimelineEngine::setLoop(bool loop)
{
    if (m_loop == loop) return;
    m_loop = loop;
    emit loopChanged(loop);
}

void TimelineEngine::rebuildActions()
{
    m_actionsModel.clear();
    for (const auto& action : m_actions) {
        QVariantMap m;
        m.insert(QStringLiteral(u"id"), action.id);
        m.insert(QStringLiteral(u"name"), action.name);
        m.insert(QStringLiteral(u"type"), action.type);
        m.insert(QStringLiteral(u"time"), action.time);
        m.insert(QStringLiteral(u"duration"), action.duration);
        m.insert(QStringLiteral(u"color"), action.color.value_or(QString{}));
        m_actionsModel.append(m);
    }
}

int TimelineEngine::findAction(const QString& id) const
{
    for (size_t i = 0; i < m_actions.size(); ++i) {
        if (m_actions[i].id == id) return int(i);
    }
    return -1;
}

// ---------------------------------------------------------------------------
// transport
// ---------------------------------------------------------------------------
void TimelineEngine::start()
{
    if (m_actions.empty()) return;
    if (m_paused) {
        m_paused = false;
    } else {
        m_position = 0;
        m_nextIndex = 0;
        m_offset = 0;
        m_clock.restart();
        emit positionChanged(0);
    }
    m_playing = true;
    m_timer.start();
    emit stateChanged();
    triggerDue(m_position, m_position);
}

void TimelineEngine::pause()
{
    if (!m_playing) return;
    m_playing = false;
    m_paused = true;
    m_timer.stop();
    emit stateChanged();
}

void TimelineEngine::stop()
{
    const bool wasActive = m_playing || m_paused;
    m_playing = false;
    m_paused = false;
    m_timer.stop();
    m_position = 0;
    m_nextIndex = 0;
    if (wasActive) {
        emit positionChanged(0);
        emit stateChanged();
    }
}

void TimelineEngine::seek(qint64 milliseconds)
{
    m_position = qMax<qint64>(0, milliseconds);
    m_offset = m_clock.elapsed() - m_position;
    m_nextIndex = 0;
    while (m_nextIndex < m_actions.size() && m_actions[m_nextIndex].time <= m_position)
        ++m_nextIndex;
    emit positionChanged(m_position);
}

void TimelineEngine::tick()
{
    if (!m_playing) return;
    const qint64 now = m_clock.elapsed() + m_offset;
    triggerDue(m_position, now);
    m_position = now;
    emit positionChanged(now);

    const qint64 end = duration();
    if (end > 0 && now >= end) {
        if (m_loop) {
            seek(0);
            m_offset = m_clock.elapsed();
            m_nextIndex = 0;
        } else {
            m_playing = false;
            m_timer.stop();
            emit stateChanged();
            emit finished();
        }
    }
}

void TimelineEngine::triggerDue(qint64 from, qint64 to)
{
    while (m_nextIndex < m_actions.size()) {
        const auto& action = m_actions[m_nextIndex];
        if (action.time > to) break;
        if (action.time >= from) emit actionTriggered(action);
        ++m_nextIndex;
    }
}

// ---------------------------------------------------------------------------
// editing
// ---------------------------------------------------------------------------
void TimelineEngine::addAction(const QString& name, const QString& type,
                               qint64 timeMs, double durationSeconds)
{
    TimelineAction action;
    action.id = makeId();
    action.name = name.isEmpty() ? QStringLiteral(u"Action"): name;
    action.type = type.isEmpty() ? QStringLiteral(u"action"): type;
    action.time = qMax<qint64>(0, timeMs);
    action.duration = qMax(0.0, durationSeconds);
    m_actions.push_back(action);
    std::stable_sort(m_actions.begin(), m_actions.end(),
                     [](const auto& a, const auto& b) { return a.time < b.time; });
    rebuildActions();
    emit actionsChanged();
}

bool TimelineEngine::removeAction(const QString& id)
{
    const int index = findAction(id);
    if (index < 0) return false;
    m_actions.erase(m_actions.begin() + index);
    rebuildActions();
    emit actionsChanged();
    return true;
}

void TimelineEngine::updateActionTime(const QString& id, qint64 timeMs)
{
    const int index = findAction(id);
    if (index < 0) return;
    m_actions[size_t(index)].time = qMax<qint64>(0, timeMs);
    std::stable_sort(m_actions.begin(), m_actions.end(),
                     [](const auto& a, const auto& b) { return a.time < b.time; });
    m_nextIndex = 0;
    while (m_nextIndex < m_actions.size() && m_actions[m_nextIndex].time <= m_position)
        ++m_nextIndex;
    rebuildActions();
    emit actionsChanged();
}

void TimelineEngine::clear()
{
    stop();
    m_actions.clear();
    rebuildActions();
    emit actionsChanged();
}

} // namespace ps::timeline
