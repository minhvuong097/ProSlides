#include "crdt/SyncSession.h"

#include <QJsonArray>

namespace ps::crdt {

SyncSession::SyncSession(Document& document, QObject* parent)
    : QObject(parent), m_document(document)
{
    connect(&m_document, &Document::operationReady,
            this, &SyncSession::onLocalOperation);
}

void SyncSession::setTransport(CrdtTransport* transport)
{
    if (m_transport) {
        disconnect(m_transport, nullptr, this, nullptr);
    }
    m_transport = transport;
    if (m_transport) {
        connect(m_transport, &CrdtTransport::peerConnected,
                this, &SyncSession::onPeerConnected);
        connect(m_transport, &CrdtTransport::messageReceived,
                this, &SyncSession::onMessage);
    }
}

VectorClock SyncSession::vectorFromJson(const QJsonObject& o) const
{
    VectorClock vc;
    const auto obj = o.value(QStringLiteral(u"vector")).toObject();
    for (auto it = obj.constBegin(); it != obj.constEnd(); ++it)
        vc.insert(it.key(), static_cast<ClockValue>(it.value().toDouble()));
    return vc;
}

QJsonObject SyncSession::vectorToJson(const VectorClock& vc) const
{
    QJsonObject o;
    for (auto it = vc.constBegin(); it != vc.constEnd(); ++it)
        o.insert(it.key(), it.value());
    return o;
}

void SyncSession::onPeerConnected(const QString& peerId)
{
    m_helloSent.insert(peerId);
    if (!m_transport) return;
    QJsonObject hello;
    hello.insert(QStringLiteral(u"type"), QStringLiteral(u"hello"));
    hello.insert(QStringLiteral(u"actor"), m_document.actorId());
    hello.insert(QStringLiteral(u"vector"), vectorToJson(m_document.vectorClock()));
    m_transport->sendToPeer(peerId, hello);
}

void SyncSession::requestSync()
{
    if (!m_transport) return;
    QJsonObject request;
    request.insert(QStringLiteral(u"type"), QStringLiteral(u"sync-request"));
    request.insert(QStringLiteral(u"vector"), vectorToJson(m_document.vectorClock()));
    m_transport->broadcast(request);
}

void SyncSession::onLocalOperation(const Operation& op)
{
    if (!m_transport) return;
    QJsonObject msg;
    msg.insert(QStringLiteral(u"type"), QStringLiteral(u"ops"));
    msg.insert(QStringLiteral(u"actor"), m_document.actorId());
    QJsonArray arr;
    arr.append(op.toJson());
    msg.insert(QStringLiteral(u"ops"), arr);
    msg.insert(QStringLiteral(u"vector"), vectorToJson(m_document.vectorClock()));
    m_transport->broadcast(msg);
}

void SyncSession::onMessage(const QString& peerId, const QJsonObject& message)
{
    if (!m_transport) return;
    const QString type = message.value(QStringLiteral(u"type")).toString();

    if (type == QStringLiteral(u"hello")) {
        const VectorClock peerClock = vectorFromJson(message);
        const auto missing = m_document.operationsSince(peerClock);
        if (!missing.isEmpty()) {
            QJsonObject reply;
            reply.insert(QStringLiteral(u"type"), QStringLiteral(u"ops"));
            reply.insert(QStringLiteral(u"actor"), m_document.actorId());
            QJsonArray arr;
            for (const auto& op : missing) arr.append(op.toJson());
            reply.insert(QStringLiteral(u"ops"), arr);
            reply.insert(QStringLiteral(u"vector"), vectorToJson(m_document.vectorClock()));
            m_transport->sendToPeer(peerId, reply);
        }
        emit synced(peerId);
        return;
    }

    if (type == QStringLiteral(u"ops")|| type == QStringLiteral(u"sync-request")) {
        if (type == QStringLiteral(u"sync-request")) {
            const VectorClock peerClock = vectorFromJson(message);
            const auto missing = m_document.operationsSince(peerClock);
            if (!missing.isEmpty()) {
                QJsonObject reply;
                reply.insert(QStringLiteral(u"type"), QStringLiteral(u"ops"));
                reply.insert(QStringLiteral(u"actor"), m_document.actorId());
                QJsonArray arr;
                for (const auto& op : missing) arr.append(op.toJson());
                reply.insert(QStringLiteral(u"ops"), arr);
                reply.insert(QStringLiteral(u"vector"), vectorToJson(m_document.vectorClock()));
                m_transport->sendToPeer(peerId, reply);
            }
            return;
        }

        QVector<Operation> incoming;
        const auto arr = message.value(QStringLiteral(u"ops")).toArray();
        for (const auto& v : arr) {
            if (v.isObject()) incoming.append(Operation::fromJson(v.toObject()));
        }
        m_document.applyBatch(incoming);
        m_document.mergeVectorClock(vectorFromJson(message));
    }
}

} // namespace ps::crdt
