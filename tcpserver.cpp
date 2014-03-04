#include <QTcpSocket>

#include "tcpclient.h"
#include "tcpserver.h"

TcpServer * TcpServer::m_Instance = NULL;
static const char headDebug[] = "[TcpServer]";

TcpServer * TcpServer::instance (QObject * parent) {
    if (m_Instance == NULL) {
        new TcpServer (parent);
    }

    return m_Instance;
}

TcpServer::TcpServer (QObject * parent) : QTcpServer (parent) {
    qDebug() << __FILE__ << __LINE__ << __func__;

    m_Instance = this;
    m_client = NULL;
    m_debug = false;
}

TcpServer::~TcpServer() {
    m_Instance = NULL;
}

void TcpServer::debug (const QString &testo)
{
    if (m_debug) {
        qDebug() << headDebug << qPrintable(testo);
    }
}

/*!
* \brief TcpServer::deleteClient - Metodo per cancellare il client e il relativo socket
*/
void TcpServer::deleteClient(QTcpSocket *socket) {
    qDebug() << __FILE__ << __LINE__ << __func__;

    if (m_client->getSocket() == socket) {
        disconnect(m_client);
        disconnect(socket);
        delete m_client;
        socket->deleteLater();
    }
}

/*************************************************************
*
*                        GET/SET
*
*
*************************************************************/

void TcpServer::setDebug (const bool &val)
{
    m_debug = val;
}

/*************************************************************
*
*                        SLOTS
*
*
*************************************************************/

/*!
* \brief TcpServer::newConnectionSlot
*  Tutte le nuovi connessioni da parte dei clients vengono gestite qua.
*  Vengono inserite dentro la lista m_clients in modo che quando dovro' mandare un messaggio
*  avro' un riferimento.
*/
void TcpServer::newConnectionSlot() {
    qDebug() << __FILE__ << __LINE__ << __func__;

    while (hasPendingConnections()) {
        QTcpSocket *socket = nextPendingConnection();
        if (socket) {
            if (m_client == NULL) {
                TcpClient *client = new TcpClient(this);
                client->setSocket(socket);
                client->setDebug(m_debug);
                m_client = client;

                // Quando c'e' qualche errore nel network
                connect (socket, SIGNAL(error(QAbstractSocket::SocketError)),
                         this, SLOT(erroSocketSlot(QAbstractSocket::SocketError)));
            }
            else
            {
                debug("One Client already connected.");
                delete socket;
            }
        }
    }
}


/*!
* \brief TcpServer::erroSocketSlot - Slot per gestire gli errori del socket
*/
void TcpServer::erroSocketSlot(QAbstractSocket::SocketError) {
    qDebug() << __FILE__ << __LINE__ << __func__;
    QTcpSocket *socket = (QTcpSocket *) sender();
    QString testo = QString ("Client %1 error").arg(socket->peerAddress().toString());
    debug(testo);
    deleteClient(socket);
}
