#pragma once

#include "Protocol.h"
#include "crdt/SyncSession.h"

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>
#include <QMap>

namespace ps::remote {

// WebSocket server speaking the FreeShow-style REMOTE protocol on port 5510.
// It doubles as the CRDT transport (channel "CRDT").
class RemoteServer : public crdt::CrdtTransport
{
    Q_OBJECT

public:
    explicit RemoteServer(QObject* parent = nullptr);
    ~RemoteServer() override;

    bool start(quint16 port = 5510, QString password = {}, int maxClients = 10);
    void stop();
    bool isRunning() const;
    quint16 port() const;
    int clientCount() const { return m_clients.size(); }

    void setPassword(const QString& password) { m_password = password; }

    // Push state snapshots to every authenticated remote client.
    void publishShows(const QJsonArray& shows);
    void publishShow(const QJsonObject& show, const QString& id);
    void publishProjects(const QJsonArray& projects);
    void publishOutput(const QJsonObject& output);
    void publishOutputData(const QJsonObject& data);
    void broadcastMessage(const Message& message);

    // Reply to an API call, sent only to the requesting client.
    void reply(const QString& clientId, const QString& api, const QJsonValue& result);

public slots:
    void sendToPeer(const QString& peerId, const QJsonObject& message) override;
    void broadcast(const QJsonObject& message) override;

signals:
    void apiRequested(const QString& api, const QJsonObject& data,
                      const QString& clientId, bool wantsResponse);
    void clientAuthenticated(const QString& clientId);
    void clientDisconnected(const QString& clientId);
    void errorOccurred(const QString& error);

private slots:
    void onNewConnection();
    void onTextMessage(const QString& text);
    void onSocketDisconnected();

private:
    void sendTo(QWebSocket* socket, const Message& message);
    void sendTo(QWebSocket* socket, const QJsonObject& obj);
    QWebSocket* socketFor(const QString& clientId) const;
    QString clientIdFor(QWebSocket* socket) const;

    QWebSocketServer* m_server = nullptr;
    QMap<QString, QWebSocket*> m_clients;      // clientId -> socket
    QMap<QWebSocket*, QString> m_ids;          // socket -> clientId
    QMap<QWebSocket*, bool> m_authenticated;
    QString m_password;
    int m_maxClients = 10;
};

} // namespace ps::remote
