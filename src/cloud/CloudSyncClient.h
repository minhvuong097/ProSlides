#pragma once

#include "crdt/SyncSession.h"

#include <QObject>
#include <QWebSocket>
#include <QUrl>

namespace ps::cloud {

// CRDT transport over a WebSocket relay server.
class RelayTransport : public crdt::CrdtTransport
{
    Q_OBJECT
public:
    explicit RelayTransport(QObject* parent = nullptr);

    void connectTo(const QUrl& url, const QString& room, const QString& token);
    void disconnectFromServer();
    bool connected() const { return m_socket.state() == QAbstractSocket::ConnectedState; }

    void sendToPeer(const QString& peerId, const QJsonObject& message) override;
    void broadcast(const QJsonObject& message) override;

signals:
    void connectedChanged(bool connected);
    void errorOccurred(const QString& error);

private slots:
    void onConnected();
    void onTextMessage(const QString& text);
    void onDisconnected();

private:
    QWebSocket m_socket;
    QString m_room;
    QString m_token;
};

// Owns a SyncSession bound to the shared document and the relay transport.
class CloudSyncClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)

public:
    explicit CloudSyncClient(crdt::Document& document, QObject* parent = nullptr);

    bool connected() const { return m_transport.connected(); }

    Q_INVOKABLE void connectTo(const QString& url, const QString& room,
                               const QString& token = {});
    Q_INVOKABLE void disconnectFromServer();
    Q_INVOKABLE void requestSync() { m_sync.requestSync(); }

signals:
    void connectedChanged(bool connected);
    void errorOccurred(const QString& error);

private:
    RelayTransport m_transport;
    crdt::SyncSession m_sync;
};

} // namespace ps::cloud
