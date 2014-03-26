#include <QDebug>

#include "bufferize.h"
#include "udpserver.h"
static const char headDebug[] = "[SlaveUdpClient]";

UdpServer *UdpServer::m_Instance = NULL;

UdpServer *UdpServer::instance(QObject *parent) {
    if (m_Instance == NULL) {
        new UdpServer(parent);
    }

    return m_Instance;
}

UdpServer::UdpServer(QObject *parent) :
    QUdpSocket(parent) {
    m_Instance = this;
    m_debug = false;
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

        if (m_debug)
        {
            QDebug debugBuffer = qDebug();
            debugBuffer << headDebug << "Rx ";
            quint8 var;
            foreach (var, datagram) {
                debugBuffer << hex << var;
            }
        }
    }
}
