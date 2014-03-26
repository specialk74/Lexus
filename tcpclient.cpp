#include <QTcpSocket>
#include <QNetworkInterface>

#include "bufferize.h"
#include "tcpclient.h"
#include "serialdev.h"

static const char headDebug[] = "[SlaveTcpClient]";

TcpClient::TcpClient(QObject *parent) :
    QObject(parent) {    
    m_debug = NULL;
    m_socket = NULL;
    connect (SerialDev::instance(), SIGNAL(dataFromDevice(QByteArray)),
             this, SLOT(send(QByteArray)));
}

void TcpClient::debug (const QString &testo)
{
    if (m_debug) {
        qDebug() << headDebug << qPrintable(testo);
    }
}

TcpClient::~TcpClient() {
    debug("DTor");
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

    Bufferize::instance()->addBuffer(buffer, this);
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

void TcpClient::sendMsg(const QByteArray &data) {
//    qDebug() << __FILE__ << __LINE__ << __func__ << m_socket;

    QByteArray buffer;
    buffer.append(DLE);
    buffer.append(STX);
    quint8 var;
    foreach (var, data) {
        buffer.append(var);
        if (var == DLE) {
            buffer.append(DLE);
        }
    }
    buffer.append(DLE);
    buffer.append(ETX);

    if (m_debug) {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Tx ";
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }

    m_socket->write(buffer);
}

void TcpClient::setSocket (QTcpSocket *socket) {
    m_socket = socket;
    connect (socket, SIGNAL(readyRead()), this, SLOT(fromClientsSlot()));

}

QTcpSocket * TcpClient::getSocket () {
    return m_socket;
}
