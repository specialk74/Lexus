#include "bufferize.h"
#include "udpserver.h"

UdpServer *UdpServer::m_Instance = NULL;

UdpServer *UdpServer::instance(QObject *parent) {
    if (m_Instance == NULL) {
        new UdpServer(parent);
    }

    return m_Instance;
}

UdpServer::UdpServer(QObject *parent) :
    QUdpSocket(parent)
{
    m_Instance = this;
    connect(this, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}

UdpServer::~UdpServer () {
    m_Instance = NULL;
}

void UdpServer::readPendingDatagrams() {
    while (hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(pendingDatagramSize());

        readDatagram(datagram.data(), datagram.size());

        Bufferize::instance()->addBuffer(datagram);
    }
}
