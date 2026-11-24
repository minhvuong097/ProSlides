#include "Protocol.h"

#include <QJsonDocument>

namespace ps::remote {

QJsonObject toJson(const Message& m)
{
    QJsonObject o;
    o.insert(u"channel", m.channel);
    o.insert(u"data", m.data);
    if (!m.id.isEmpty()) o.insert(u"id", m.id);
    return o;
}

Message fromJson(const QJsonObject& o)
{
    Message m;
    m.channel = o.value(u"channel").toString();
    m.data = o.value(u"data");
    m.id = o.value(u"id").toString();
    return m;
}

QByteArray serialize(const Message& m)
{
    return QJsonDocument(toJson(m)).toJson(QJsonDocument::Compact);
}

ApiCall apiCallFromJson(const QJsonObject& o)
{
    ApiCall call;
    call.api = o.value(u"api").toString();
    call.data = o.value(u"data").toObject();
    call.id = o.value(u"id").toString();
    call.wantsResponse = o.value(u"send").toBool(false);
    return call;
}

QJsonObject apiCallToJson(const ApiCall& call)
{
    QJsonObject o;
    o.insert(u"channel", Channels::Api);
    o.insert(u"api", call.api);
    o.insert(u"data", call.data);
    if (!call.id.isEmpty()) o.insert(u"id", call.id);
    if (call.wantsResponse) o.insert(u"send", true);
    return o;
}

} // namespace ps::remote
