#include "Operation.h"

#include <QJsonArray>

namespace ps::crdt {

QJsonObject Operation::toJson() const
{
    QJsonObject o;
    o.insert(u"t", id.time);
    o.insert(u"a", id.actor);
    o.insert(u"k", static_cast<int>(kind));
    QJsonArray p;
    for (const auto& x : path) p.append(x);
    o.insert(u"p", p);
    if (!value.isUndefined()) o.insert(u"v", value);
    if (!parent.actor.isEmpty()) {
        QJsonObject po;
        po.insert(u"t", parent.time);
        po.insert(u"a", parent.actor);
        o.insert(u"par", po);
    }
    if (!target.actor.isEmpty()) {
        QJsonObject to;
        to.insert(u"t", target.time);
        to.insert(u"a", target.actor);
        o.insert(u"tar", to);
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
    id.time = static_cast<ClockValue>(obj.value(u"t").toDouble());
    id.actor = obj.value(u"a").toString();
    return id;
}
} // namespace

Operation Operation::fromJson(const QJsonObject& o)
{
    Operation op;
    op.id.time = static_cast<ClockValue>(o.value(u"t").toDouble());
    op.id.actor = o.value(u"a").toString();
    op.kind = static_cast<OpKind>(o.value(u"k").toInt(0));
    const auto p = o.value(u"p").toArray();
    for (const auto& x : p) op.path << x.toString();
    if (o.contains(u"v")) op.value = o.value(u"v");
    op.parent = readId(o, u"par");
    op.target = readId(o, u"tar");
    return op;
}

} // namespace ps::crdt
