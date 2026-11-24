#include "Protocol.h"

#include <QJsonDocument>

namespace ps::remote {

QJsonObject toJson(const Message& m)
{
    QJsonObject o;
    o.insert(QStringLiteral(u"channel"), m.channel);
    o.insert(QStringLiteral(u"data"), m.data);
    if (!m.id.isEmpty()) o.insert(QStringLiteral(u"id"), m.id);
    return o;
}

Message fromJson(const QJsonObject& o)
{
    Message m;
    m.channel = o.value(QStringLiteral(u"channel")).toString();
    m.data = o.value(QStringLiteral(u"data"));
    m.id = o.value(QStringLiteral(u"id")).toString();
    return m;
}

QByteArray serialize(const Message& m)
{
    return QJsonDocument(toJson(m)).toJson(QJsonDocument::Compact);
}

ApiCall apiCallFromJson(const QJsonObject& o)
{
    ApiCall call;
    call.api = o.value(QStringLiteral(u"api")).toString();
    call.data = o.value(QStringLiteral(u"data")).toObject();
    call.id = o.value(QStringLiteral(u"id")).toString();
    call.wantsResponse = o.value(QStringLiteral(u"send")).toBool(false);
    return call;
}

QJsonObject apiCallToJson(const ApiCall& call)
{
    QJsonObject o;
    o.insert(QStringLiteral(u"channel"), Channels::Api);
    o.insert(QStringLiteral(u"api"), call.api);
    o.insert(QStringLiteral(u"data"), call.data);
    if (!call.id.isEmpty()) o.insert(QStringLiteral(u"id"), call.id);
    if (call.wantsResponse) o.insert(QStringLiteral(u"send"), true);
    return o;
}

} // namespace ps::remote
