#include "CloudSyncClient.h"

#include <QJsonDocument>

namespace ps::cloud {

RelayTransport::RelayTransport(QObject* parent)
    : crdt::CrdtTransport(parent)
{
    connect(&m_socket, &QWebSocket::connected, this, &RelayTransport::onConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &RelayTransport::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &RelayTransport::onTextMessage);
    connect(&m_socket, &QWebSocket::errorOccurred, this, [this]() {
        emit errorOccurred(m_socket.errorString());
    });
}

void RelayTransport::connectTo(const QUrl& url, const QString& room, const QString& token)
{
    m_room = room;
    m_token = token;
    m_socket.open(url);
}

void RelayTransport::disconnectFromServer()
{
    m_socket.close();
}

void RelayTransport::onConnected()
{
    QJsonObject join;
    join.insert(QStringLiteral(u"type"), QStringLiteral(u"join"));
    join.insert(QStringLiteral(u"room"), m_room);
    join.insert(QStringLiteral(u"token"), m_token);
    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(join).toJson(QJsonDocument::Compact)));
    emit connectedChanged(true);
}

void RelayTransport::onDisconnected()
{
    emit connectedChanged(false);
}

void RelayTransport::onTextMessage(const QString& text)
{
    const auto doc = QJsonDocument::fromJson(text.toUtf8());
    if (!doc.isObject()) return;
    const auto obj = doc.object();
    const QString type = obj.value(QStringLiteral(u"type")).toString();

    if (type == QStringLiteral(u"peer")) {
        emit peerConnected(obj.value(QStringLiteral(u"id")).toString());
        return;
    }
    if (type == QStringLiteral(u"leave")) {
        emit peerDisconnected(obj.value(QStringLiteral(u"id")).toString());
        return;
    }
    if (type == QStringLiteral(u"message")) {
        const QString from = obj.value(QStringLiteral(u"from")).toString();
        emit messageReceived(from, obj.value(QStringLiteral(u"payload")).toObject());
    }
}

void RelayTransport::sendToPeer(const QString& peerId, const QJsonObject& message)
{
    if (!connected()) return;
    QJsonObject o;
    o.insert(QStringLiteral(u"type"), QStringLiteral(u"message"));
    o.insert(QStringLiteral(u"to"), peerId);
    o.insert(QStringLiteral(u"payload"), message);
    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));
}

void RelayTransport::broadcast(const QJsonObject& message)
{
    if (!connected()) return;
    QJsonObject o;
    o.insert(QStringLiteral(u"type"), QStringLiteral(u"message"));
    o.insert(QStringLiteral(u"payload"), message);
    m_socket.sendTextMessage(QString::fromUtf8(QJsonDocument(o).toJson(QJsonDocument::Compact)));
}

// ---------------------------------------------------------------------------

CloudSyncClient::CloudSyncClient(crdt::Document& document, QObject* parent)
    : QObject(parent), m_sync(document)
{
    m_sync.setTransport(&m_transport);
    connect(&m_transport, &RelayTransport::connectedChanged,
            this, &CloudSyncClient::connectedChanged);
    connect(&m_transport, &RelayTransport::errorOccurred,
            this, &CloudSyncClient::errorOccurred);
}

void CloudSyncClient::connectTo(const QString& url, const QString& room, const QString& token)
{
    m_transport.connectTo(QUrl(url), room, token);
}

void CloudSyncClient::disconnectFromServer()
{
    m_transport.disconnectFromServer();
}

} // namespace ps::cloud
