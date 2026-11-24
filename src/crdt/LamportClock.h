#pragma once

#include "Operation.h"

#include <QMap>

namespace ps::crdt {

using VectorClock = QMap<QString, ClockValue>;

// A hybrid logical/Lamport clock: a scalar counter plus a per-actor vector
// clock. The vector clock is used to skip ops a peer has already seen.
class LamportClock
{
public:
    explicit LamportClock(QString actorId = {}) : m_actor(std::move(actorId)) {}

    const QString& actor() const { return m_actor; }
    void setActor(QString actor) { m_actor = std::move(actor); }

    ClockValue value() const { return m_time; }
    OpId tick()
    {
        ++m_time;
        m_vector[m_actor] = m_time;
        return { m_time, m_actor };
    }

    void observe(const OpId& id)
    {
        if (id.time > m_time) m_time = id.time;
        if (id.time > m_vector.value(id.actor, 0)) m_vector[id.actor] = id.time;
    }

    void observe(const VectorClock& other)
    {
        for (auto it = other.constBegin(); it != other.constEnd(); ++it) {
            if (it.value() > m_vector.value(it.key(), 0)) m_vector[it.key()] = it.value();
            if (it.value() > m_time) m_time = it.value();
        }
    }

    const VectorClock& vector() const { return m_vector; }
    void setVector(VectorClock v)
    {
        m_vector = std::move(v);
        m_time = 0;
        for (const auto value : m_vector) if (value > m_time) m_time = value;
    }

private:
    QString m_actor;
    ClockValue m_time = 0;
    VectorClock m_vector;
};

} // namespace ps::crdt
