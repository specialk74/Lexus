#include <QDebug>
#include <QTimer>
#include "bufferize.h"
#include "serialdev.h"
#include "tcpclient.h"
#include "tcpserver.h"


#define VERSIONE_SW_UNITA         1
#define VERSIONE_SW_DECIMALI      0

static const char headDebug[] = "[SlaveBufferize]";

Bufferize *Bufferize::m_Instance = NULL;


Bufferize * Bufferize::instance (QObject *parent) {
    if (m_Instance == NULL) {
        new Bufferize(parent);
    }


    return m_Instance;
}

Bufferize::~Bufferize () {
    m_Instance = NULL;
}

Bufferize::Bufferize (QObject *parent) : QObject (parent) {
    m_Instance 			= this;
    m_idxAddSequence 	= 0;
    m_idxGetSequence 	= 0;
    m_idxAddInstantCmd 	= 0;
    m_idxGetInstantCmd 	= 0;
    m_dimVector			= 0;
    m_statoParser       = STATO_DLE_STX;

    timeoutConn = 100;

    timerSec = new QTimer (this);
    connect (timerSec, SIGNAL(timeout()), this, SLOT(timer1Sec()));
    timerSec->start (1000);
}

void TcpServer::timer1Sec (void)
{
    sec++;

    if ((sec % 10) == 0) {
        //client->sendGetId ();
    }/* endif */

    timeoutConn--;
    if (timeoutConn == 0) {
#ifdef Q_WS_QWS
        system ("reboot");
#endif // #ifdef Q_WS_QWS
    }/* endif */
}

void Bufferize::addBuffer(const QByteArray &buffer, TcpClient *client) {
    int idx = 0;
/*
    c "Bufferize::addBuffer  start:" << idx <<
                "  buffer.length(): " << buffer.length() << "   ";
                */
    // Fin tanto che non sono arrivato al fondo del buffer, decodifico!
    while (idx < buffer.length()) {
        if (decodeMessage (buffer, m_bufferDest, idx, m_statoParser)) {
            addSingleBuffer(m_bufferDest, client);
            // Ripulisco il buffer perche' gia' gestito
            m_bufferDest.clear();
        }
    }
    SerialDev::instance()->start();
}

void Bufferize::addSingleBuffer(const QByteArray &buffer, TcpClient *client) {
    if (m_dimVector == 0) {
        qDebug() << "Dim Vector is 0";
        return;
    }
    if (buffer.length() < 4) {
        qDebug() << "Dim Buffer is < 4";
        return;
    }

    timeoutConn = 20;

    switch (buffer.at(2)) {
    case TYPE_SYNC:
        m_Sync = buffer;
        break;
    case TYPE_BUFFER_ALL_OUTPUT_DIGITAL:
    case TYPE_BUFFER_OUTPUT_DIGITAL:
        m_Sequence[m_idxAddSequence] = buffer;
        m_idxAddSequence++;
        m_idxAddSequence %= m_dimVector;
        break;
    default:
        if (client) {
            if (buffer.at(2) == TYPE_GET_ID) { // invio ID
                QByteArray sendBuffer;

                sendBuffer.append ((char) TYPE_ID_MODULO);
                sendBuffer.append ((char) 0);// [1] = 0;
                sendBuffer.append ((char) 6); // len
                sendBuffer.append ((char) SerialDev::instance()->getIpAddress()); // tipo oggetto
                sendBuffer.append ((char) VERSIONE_SW_UNITA); // ver SW
                sendBuffer.append ((char) VERSIONE_SW_DECIMALI); // ver SW
                client->sendMsg (sendBuffer);
            } else if (buffer.at(2) == TYPE_CMD_POWER_OFF) {
                qDebug() << "power off:";
                QTimer::singleShot(1000, this, SLOT(powerOff()));

            } else if (buffer.at(2) == TYPE_CMD_REBOOT) {
                qDebug() << "power off:";
                QTimer::singleShot(1000, this, SLOT(reboot()));

            }
        }
        m_InstantCmd [m_idxAddInstantCmd] = buffer;
        m_idxAddInstantCmd++;
        m_idxAddInstantCmd %= m_dimVector;
        break;
    }
}

void Bufferize::powerOff (void) {
#ifdef Q_WS_QWS
    //                    PowerManager::instance()->setOutput('0'); // spegne la scheda IO
    delete TcpServer::instance();
    system ("ifconfig wlan0 down");
    system ("poweroff");
#endif // #ifdef Q_WS_QWS
}

void Bufferize::reboot (void) {
#ifdef Q_WS_QWS
    //                    PowerManager::instance()->setOutput('0'); // spegne la scheda IO
    delete TcpServer::instance();
    system ("ifconfig wlan0 down");
    system ("reboot");
#endif // #ifdef Q_WS_QWS
}

bool Bufferize::getBuffer(QByteArray &buffer) {
    if (m_dimVector == 0) {
        return false;
    }

    if (m_Sync.length() != 0) {
        buffer = m_Sync;
        m_Sync.clear();
        return true;
    }
    else if (m_idxAddInstantCmd != m_idxGetInstantCmd) {
        buffer = m_InstantCmd[m_idxGetInstantCmd];
        m_idxGetInstantCmd++;
        m_idxGetInstantCmd %= m_dimVector;
        return true;
    }
    else if (m_idxAddSequence != m_idxGetSequence) {
        buffer = m_Sequence[m_idxGetSequence];
        m_idxGetSequence++;
        m_idxGetSequence %= m_dimVector;
        return true;
    }

    return false;
}

void Bufferize::setVector (const quint32 &dim) {
    m_dimVector = dim;
    m_Sequence.resize (dim);
    m_InstantCmd.resize (dim);
}
