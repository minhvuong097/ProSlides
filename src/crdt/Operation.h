#pragma once

#include <QString>
#include <QStringList>
#include <QJsonValue>
#include <QJsonObject>

#include <cstdint>

namespace ps::crdt {

using ClockValue = qint64;

// Globally unique, causally ordered operation id (Lamport timestamp + actor).
struct OpId
{
    ClockValue time = 0;
    QString actor;

    friend bool operator<(const OpId& a, const OpId& b)
    {
        if (a.time != b.time) return a.time < b.time;
        return a.actor < b.actor;
    }
    friend bool operator==(const OpId& a, const OpId& b) = default;

    QString toString() const { return QString::number(time, 16) + u':' + actor; }
};

enum class OpKind : quint8 {
    SetField = 0,   // LWW register at path
    DeleteField,
    SeqInsert,      // insert value into sequence at path after `parent`
    SeqDelete,      // tombstone element `target`
    SeqMove         // move element `target` after `parent`
};

struct Operation
{
    OpId id;
    OpKind kind = OpKind::SetField;
    QStringList path;        // JSON pointer-like path, e.g. ["slides","s1","notes"]
    QJsonValue value;        // payload for SetField / SeqInsert
    OpId parent;             // causal predecessor for sequence ops
    OpId target;             // element to delete/move

    QJsonObject toJson() const;
    static Operation fromJson(const QJsonObject& o);
};

} // namespace ps::crdt
