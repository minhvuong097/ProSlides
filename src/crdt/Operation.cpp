#include "Operation.h"

#include <QJsonArray>

namespace ps::crdt {

QJsonObject Operation::toJson() const
{
    QJsonObject o;
    o.insert(QStringLiteral(u"t"), id.time);
    o.insert(QStringLiteral(u"a"), id.actor);
    o.insert(QStringLiteral(u"k"), static_cast<int>(kind));
    QJsonArray p;
    for (const auto& x : path) p.append(x);
    o.insert(QStringLiteral(u"p"), p);
    if (!value.isUndefined()) o.insert(QStringLiteral(u"v"), value);
    if (!parent.actor.isEmpty()) {
        QJsonObject po;
        po.insert(QStringLiteral(u"t"), parent.time);
        po.insert(QStringLiteral(u"a"), parent.actor);
        o.insert(QStringLiteral(u"par"), po);
    }
    if (!target.actor.isEmpty()) {
        QJsonObject to;
        to.insert(QStringLiteral(u"t"), target.time);
        to.insert(QStringLiteral(u"a"), target.actor);
        o.insert(QStringLiteral(u"tar"), to);
    }
    return o;
}

namespace {
OpId readId(const QJsonObject& o, const QString& key)
{
    OpId id;
    const auto it = o.constFind(key);
    if (it == o.constEnd() || !it->isObject()) return id;
    const auto obj = it->toObject();
    id.time = static_cast<ClockValue>(obj.value(QStringLiteral(u"t")).toDouble());
    id.actor = obj.value(QStringLiteral(u"a")).toString();
    return id;
}
} // namespace

Operation Operation::fromJson(const QJsonObject& o)
{
    Operation op;
    op.id.time = static_cast<ClockValue>(o.value(QStringLiteral(u"t")).toDouble());
    op.id.actor = o.value(QStringLiteral(u"a")).toString();
    op.kind = static_cast<OpKind>(o.value(QStringLiteral(u"k")).toInt(0));
    const auto p = o.value(QStringLiteral(u"p")).toArray();
    for (const auto& x : p) op.path << x.toString();
    if (o.contains(QStringLiteral(u"v"))) op.value = o.value(QStringLiteral(u"v"));
    op.parent = readId(o, QStringLiteral(u"par"));
    op.target = readId(o, QStringLiteral(u"tar"));
    return op;
}

} // namespace ps::crdt
