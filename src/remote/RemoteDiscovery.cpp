#include "RemoteDiscovery.h"

#include <QJsonDocument>
#include <QNetworkInterface>
#include <QHostAddress>

namespace ps::remote {

RemoteDiscovery::RemoteDiscovery(QObject* parent)
    : QObject(parent)
{
    m_timer.setInterval(5000);
    connect(&m_timer, &QTimer::timeout, this, &RemoteDiscovery::broadcast);
    connect(&m_socket, &QUdpSocket::readyRead, this, &RemoteDiscovery::readPending);
}

void RemoteDiscovery::start(quint16 servicePort, const QString& name)
{
    m_servicePort = servicePort;
    m_name = name;
    m_socket.bind(QHostAddress::AnyIPv4, 5514, QUdpSocket::ShareAddress);
    m_timer.start();
    broadcast();
}

void RemoteDiscovery::stop()
{
    m_timer.stop();
    m_socket.close();
}

void RemoteDiscovery::broadcast()
{
    QJsonObject o;
    o.insert(QStringLiteral(u"app"), QStringLiteral(u"ProSlides"));
    o.insert(QStringLiteral(u"name"), m_name);
    o.insert(QStringLiteral(u"port"), m_servicePort);
    o.insert(QStringLiteral(u"version"), QStringLiteral(u"0.9.0"));
    const auto data = QJsonDocument(o).toJson(QJsonDocument::Compact);
    m_socket.writeDatagram(data, QHostAddress(QStringLiteral(u"255.255.255.255")), 5514);
}

void RemoteDiscovery::readPending()
{
    while (m_socket.hasPendingDatagrams()) {
        QByteArray data;
        data.resize(int(m_socket.pendingDatagramSize()));
        QHostAddress sender;
        quint16 port = 0;
        m_socket.readDatagram(data.data(), data.size(), &sender, &port);
        const auto obj = QJsonDocument::fromJson(data).object();
        if (obj.value(QStringLiteral(u"app")).toString() != QStringLiteral(u"ProSlides")) continue;
        if (obj.value(QStringLiteral(u"name")).toString() == m_name) continue;

        RemotePeer peer;
        peer.name = obj.value(QStringLiteral(u"name")).toString();
        peer.host = sender.isLoopback() ? QStringLiteral(u"127.0.0.1"): sender.toString();
        peer.port = quint16(obj.value(QStringLiteral(u"port")).toInt());
        peer.version = obj.value(QStringLiteral(u"version")).toString();
        emit peerDiscovered(peer);
    }
}

} // namespace ps::remote
