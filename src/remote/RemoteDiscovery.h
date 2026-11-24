#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QTimer>
#include <QJsonObject>

namespace ps::remote {

struct RemotePeer {
    QString name;
    QString host;
    quint16 port = 0;
    QString version;
};

// Lightweight LAN discovery using UDP broadcast on port 5514.
// Lets the phone remote find the desktop app without manual IP entry.
class RemoteDiscovery : public QObject
{
    Q_OBJECT
public:
    explicit RemoteDiscovery(QObject* parent = nullptr);

    void start(quint16 servicePort, const QString& name = u"ProSlides");
    void stop();

signals:
    void peerDiscovered(const RemotePeer& peer);

private slots:
    void broadcast();
    void readPending();

private:
    QUdpSocket m_socket;
    QTimer m_timer;
    quint16 m_servicePort = 5510;
    QString m_name;
};

} // namespace ps::remote
