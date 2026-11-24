#include "RemoteServer.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QHostAddress>
#include <QUuid>

namespace ps::remote {

RemoteServer::RemoteServer(QObject* parent)
    : crdt::CrdtTransport(parent)
{
}

RemoteServer::~RemoteServer()
{
    stop();
}

bool RemoteServer::start(quint16 port, QString password, int maxClients)
{
    if (m_server) return true;
    m_password = std::move(password);
    m_maxClients = maxClients;

    m_server = new QWebSocketServer(QStringLiteral(u"ProSlides Remote"), QWebSocketServer::NonSecureMode, this);
    if (!m_server->listen(QHostAddress::Any, port)) {
        emit errorOccurred(m_server->errorString());
        delete m_server;
        m_server = nullptr;
        return false;
    }

    connect(m_server, &QWebSocketServer::newConnection, this, &RemoteServer::onNewConnection);
    return true;
}

void RemoteServer::stop()
{
    if (!m_server) return;
    m_server->close();
    for (auto* socket : m_clients) socket->close();
    m_clients.clear();
    m_ids.clear();
    m_authenticated.clear();
    delete m_server;
    m_server = nullptr;
}

bool RemoteServer::isRunning() const
{
    return m_server && m_server->isListening();
}

quint16 RemoteServer::port() const
{
    return m_server ? m_server->serverPort() : 0;
}

void RemoteServer::onNewConnection()
{
    auto* socket = m_server->nextPendingConnection();
    if (!socket) return;

    if (m_clients.size() >= m_maxClients) {
        Message error;
        error.channel = Channels::Error;
        error.data = QJsonObject{{QStringLiteral(u"key"), QStringLiteral(u"overLimit")}, {QStringLiteral(u"max"), m_maxClients}};
        sendTo(socket, error);
        socket->close();
        return;
    }

    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_clients.insert(id, socket);
    m_ids.insert(socket, id);
    m_authenticated.insert(socket, m_password.isEmpty());

    connect(socket, &QWebSocket::textMessageReceived, this, &RemoteServer::onTextMessage);
    connect(socket, &QWebSocket::disconnected, this, &RemoteServer::onSocketDisconnected);

    Message connection;
    connection.channel = Channels::Connection;
    connection.data = QJsonObject{{QStringLiteral(u"name"), QStringLiteral(u"ProSlides")}, {QStringLiteral(u"version"), QStringLiteral(u"0.9.0")}};
    sendTo(socket, connection);

    Message passwordChallenge;
    passwordChallenge.channel = Channels::Password;
    passwordChallenge.data = QJsonObject{
        {QStringLiteral(u"password"), !m_password.isEmpty()},
        {QStringLiteral(u"dictionary"), QJsonObject{}}
    };
    sendTo(socket, passwordChallenge);

    if (m_password.isEmpty()) emit clientAuthenticated(id);
    emit peerConnected(id);
}

void RemoteServer::onTextMessage(const QString& text)
{
    auto* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;
    const QString clientId = clientIdFor(socket);
    if (clientId.isEmpty()) return;

    const auto doc = QJsonDocument::fromJson(text.toUtf8());
    if (!doc.isObject()) return;
    const Message msg = fromJson(doc.object());

    // Unauthenticated clients may only answer the password challenge.
    if (!m_authenticated.value(socket, false)) {
        if (msg.channel == Channels::Access) {
            const QString supplied = msg.data.isObject()
                ? msg.data.toObject().value(QStringLiteral(u"password")).toString()
                : msg.data.toString();
            if (supplied == m_password) {
                m_authenticated[socket] = true;
                sendTo(socket, Message{Channels::Access, true, {}});
                QJsonObject theme;
                theme.insert(QStringLiteral(u"colors"), QJsonArray{ QStringLiteral(u"#0e0e10"), QStringLiteral(u"#1b1b1f"), QStringLiteral(u"#e8e8ea"), QStringLiteral(u"#3ddc84")});
                sendTo(socket, Message{Channels::ThemeColors, theme, {}});
                emit clientAuthenticated(clientId);
            } else {
                sendTo(socket, Message{Channels::Error,
                                        QJsonObject{{QStringLiteral(u"key"), QStringLiteral(u"wrongPass")}}, {}});
            }
        }
        return;
    }

    if (msg.channel == Channels::Crdt) {
        emit messageReceived(clientId, msg.data.toObject());
        return;
    }

    if (msg.channel == Channels::Api) {
        // Support {"channel":"API","api":...,"data":...,"send":true} and the
        // FreeShow variant {"channel":"API","data":{"api":...,"data":...}}.
        QJsonObject payload = msg.data.toObject();
        if (payload.isEmpty() && msg.data.isString())
            payload.insert(QStringLiteral(u"api"), msg.data.toString());
        const ApiCall call = apiCallFromJson(payload);
        emit apiRequested(call.api, call.data, clientId, call.wantsResponse);
        return;
    }

    // Other channels (SHOW, OUT, ...) are state requests answered by the
    // PresentationController through publish*().
    if (msg.channel == Channels::Show || msg.channel == Channels::Out ||
        msg.channel == Channels::Shows || msg.channel == Channels::Projects) {
        emit apiRequested(QStringLiteral(u"get_")+ msg.channel.toLower(), msg.data.toObject(), clientId, true);
    }
}

void RemoteServer::onSocketDisconnected()
{
    auto* socket = qobject_cast<QWebSocket*>(sender());
    if (!socket) return;
    const QString id = m_ids.take(socket);
    m_clients.remove(id);
    m_authenticated.remove(socket);
    m_ids.remove(socket);
    if (!id.isEmpty()) {
        emit peerDisconnected(id);
        emit clientDisconnected(id);
    }
}

void RemoteServer::sendTo(QWebSocket* socket, const Message& message)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
        socket->sendTextMessage(QString::fromUtf8(serialize(message)));
}

void RemoteServer::sendTo(QWebSocket* socket, const QJsonObject& obj)
{
    if (socket && socket->state() == QAbstractSocket::ConnectedState)
        socket->sendTextMessage(QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact)));
}

QWebSocket* RemoteServer::socketFor(const QString& clientId) const
{
    return m_clients.value(clientId, nullptr);
}

QString RemoteServer::clientIdFor(QWebSocket* socket) const
{
    return m_ids.value(socket, {});
}

void RemoteServer::sendToPeer(const QString& peerId, const QJsonObject& message)
{
    sendTo(socketFor(peerId), Message{Channels::Crdt, message, {}});
}

void RemoteServer::broadcast(const QJsonObject& message)
{
    broadcastMessage(Message{Channels::Crdt, message, {}});
}

void RemoteServer::broadcastMessage(const Message& message)
{
    const QByteArray payload = serialize(message);
    for (auto* socket : m_clients) {
        if (socket->state() == QAbstractSocket::ConnectedState)
            socket->sendTextMessage(QString::fromUtf8(payload));
    }
}

void RemoteServer::reply(const QString& clientId, const QString& api, const QJsonValue& result)
{
    QJsonObject o;
    o.insert(QStringLiteral(u"channel"), Channels::Api);
    o.insert(QStringLiteral(u"api"), api);
    o.insert(QStringLiteral(u"data"), result);
    o.insert(QStringLiteral(u"send"), true);
    sendTo(socketFor(clientId), o);
}

void RemoteServer::publishShows(const QJsonArray& shows)
{
    broadcastMessage(Message{Channels::Shows, shows, {}});
}
void RemoteServer::publishShow(const QJsonObject& show, const QString& id)
{
    QJsonObject data;
    data.insert(QStringLiteral(u"id"), id);
    data.insert(QStringLiteral(u"show"), show);
    broadcastMessage(Message{Channels::Show, data, {}});
}
void RemoteServer::publishProjects(const QJsonArray& projects)
{
    broadcastMessage(Message{Channels::Projects, projects, {}});
}
void RemoteServer::publishOutput(const QJsonObject& output)
{
    broadcastMessage(Message{Channels::Out, output, {}});
}
void RemoteServer::publishOutputData(const QJsonObject& data)
{
    broadcastMessage(Message{Channels::OutData, data, {}});
}

} // namespace ps::remote
