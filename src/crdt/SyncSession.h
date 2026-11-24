#pragma once

#include "Document.h"

#include <QObject>
#include <QJsonObject>

namespace ps::crdt {

// Abstract transport for CRDT messages. Implemented by the WebSocket remote
// server; can also be backed by a local loopback for tests.
class CrdtTransport : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

public slots:
    virtual void sendToPeer(const QString& peerId, const QJsonObject& message) = 0;
    virtual void broadcast(const QJsonObject& message) = 0;

signals:
    void messageReceived(const QString& peerId, const QJsonObject& message);
    void peerConnected(const QString& peerId);
    void peerDisconnected(const QString& peerId);
};

// Drives anti-entropy between a local Document and remote peers:
//   1. send "hello" with our vector clock on connect
//   2. peer answers with the operations we are missing
//   3. we send the operations the peer is missing
// Local edits are broadcast immediately via Document::operationReady.
class SyncSession : public QObject
{
    Q_OBJECT
public:
    explicit SyncSession(Document& document, QObject* parent = nullptr);

    void setTransport(CrdtTransport* transport);
    Document& document() { return m_document; }

public slots:
    void onPeerConnected(const QString& peerId);
    void onMessage(const QString& peerId, const QJsonObject& message);
    void onLocalOperation(const Operation& op);
    void requestSync();

signals:
    void synced(const QString& peerId);

private:
    VectorClock vectorFromJson(const QJsonObject& o) const;
    QJsonObject vectorToJson(const VectorClock& vc) const;

    Document& m_document;
    CrdtTransport* m_transport = nullptr;
    QSet<QString> m_helloSent;
};

} // namespace ps::crdt
