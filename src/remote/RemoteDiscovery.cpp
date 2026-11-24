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
    o.insert(u"app", u"ProSlides");
    o.insert(u"name", m_name);
    o.insert(u"port", m_servicePort);
    o.insert(u"version", u"0.9.0");
    const auto data = QJsonDocument(o).toJson(QJsonDocument::Compact);
    m_socket.writeDatagram(data, QHostAddress(u"255.255.255.255"), 5514);
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
        if (obj.value(u"app").toString() != u"ProSlides") continue;
        if (obj.value(u"name").toString() == m_name) continue;

        RemotePeer peer;
        peer.name = obj.value(u"name").toString();
        peer.host = sender.isLoopback() ? u"127.0.0.1" : sender.toString();
        peer.port = quint16(obj.value(u"port").toInt());
        peer.version = obj.value(u"version").toString();
        emit peerDiscovered(peer);
    }
}

} // namespace ps::remote
