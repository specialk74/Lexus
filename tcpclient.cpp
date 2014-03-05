#include <QTcpSocket>

#include "bufferize.h"
#include "tcpclient.h"
#include "serialdev.h"

static const char headDebug[] = "[TcpClient]";

TcpClient::TcpClient(QObject *parent) :
    QObject(parent) {
    m_debug = NULL;
    m_socket = NULL;
    connect (SerialDev::instance(), SIGNAL(dataFromDevice(QByteArray)),
             this, SLOT(send(QByteArray)));
}


/*!
* \brief TcpClient::fromClients - Slot per gestire i dati che mi arrivano dal client
*/
void TcpClient::fromClientsSlot() {
    QByteArray buffer = m_socket->readAll();
    if (m_debug)
    {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Rx ";
        quint8 var;
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }

    Bufferize::instance()->addBuffer(buffer);
}

void TcpClient::send (const QByteArray &buffer) {
    if (m_debug) {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Tx ";
        quint8 var;
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }
    m_socket->write(buffer);
}


void TcpClient::setSocket (QTcpSocket *socket) {
    m_socket = socket;
    connect (m_socket, SIGNAL(readyRead()), this, SLOT(fromClientsSlot()));
}

QTcpSocket * TcpClient::getSocket () {
    return m_socket;
}
