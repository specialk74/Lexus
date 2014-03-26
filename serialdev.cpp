#include <QDebug>
#include <QNetworkInterface>
#include "bufferize.h"
#include "serialdev.h"
#include "powermanager.h"
#include "tcpserver.h"

static const char headDebug[] = "[SlaveSerialDev]";
SerialDev * SerialDev::m_Instance = NULL;
#define ID_SCHEDA_IO (0x02)
#define POWER_OFF (0x2F)

SerialDev *SerialDev::instance(QObject *parent) {
    if (m_Instance == NULL) {
        new SerialDev(parent);
    }

    return m_Instance;
}

SerialDev::SerialDev(QObject *parent) :
    QSerialPort(parent)
{
    m_Instance      = this;
    m_debug         = false;
    m_statoParser   = STATO_DLE_STX;
    m_ipChecked     = true;
    m_ipAddress     = 0;
    m_powerOff      = true;

    connect (this, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWrittenSlot(qint64)));
}

SerialDev::~SerialDev()
{
    m_Instance = NULL;
}

void SerialDev::setDebug (const bool &val)
{
    m_debug = val;
}

void SerialDev::debug (const QString &testo)
{
    if (m_debug) {
        qDebug() << headDebug << qPrintable(testo);
    }
}

/*!
 * \brief SerialDev::configPort - Parametri per configurare la porta seriale
 * \return true se riesce a configurare correttamente la porta seriale
 */
bool SerialDev::configPort (const QString &name)
{
    bool debugVal = m_debug;
    m_debug = true;
    setPortName(name);

    if (!open(QIODevice::ReadWrite)) {
        QString testo = QString("Can't open %1, error code %2")
                    .arg(portName()).arg(error());
        debug(testo);
        return false;
    }

    if (!setBaudRate(QSerialPort::Baud115200)) {
        QString testo = QString("Can't set rate 115200 baud to port %1, error code %2")
                     .arg(portName()).arg(error());
        debug(testo);
        return false;
    }

    if (!setDataBits(QSerialPort::Data8)) {
        QString testo = QString("Can't set 8 data bits to port %1, error code %2")
                     .arg(portName()).arg(error());
        debug(testo);
        return false;
    }

    if (!setParity(QSerialPort::NoParity)) {
        QString testo = QString("Can't set no patity to port %1, error code %2")
                     .arg(portName()).arg(error());
        debug(testo);
        return false;
    }

    if (!setStopBits(QSerialPort::OneStop)) {
        QString testo = QString("Can't set 1 stop bit to port %1, error code %2")
                     .arg(portName()).arg(error());
        debug(testo);
        return false;
    }

    if (!setFlowControl(QSerialPort::NoFlowControl)) {
        QString testo = QString("Can't set no flow control to port %1, error code %2")
                     .arg(portName()).arg(error());
        debug(testo);
        return false;
    }

    connect(this, SIGNAL(error(QSerialPort::SerialPortError)),
            this, SLOT(errorSlot(QSerialPort::SerialPortError)));
    connect(this, SIGNAL(readyRead()), this, SLOT(fromDeviceSlot()));

    m_debug = debugVal;
   return true;
}

void SerialDev::sendMsg (const QByteArray &buffer) {
    if (m_debug) {
        quint8 var;
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Tx ";
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }
    write(buffer);
//    flush();
}

void SerialDev::start () {
    QByteArray buffer;
    if ((bytesToWrite() == 0) && Bufferize::instance()->getBuffer(buffer)) {
        sendMsg (buffer);
    }
}

/*************************************************************
 *
 *                        SLOTS
 *
 *
 *************************************************************/
/*!
 * \brief Rs232DevicePrivate::errorSlot - Gestisce se viene scollegato il converter
 * \param serialPortError
 */
void SerialDev::errorSlot(QSerialPort::SerialPortError serialPortError) {
    if (m_debug) {
        qDebug() << "Error" << serialPortError;
    }

    switch (serialPortError) {
        case QSerialPort::NoError:
            // Non faccio nulla
            break;

        case QSerialPort::ResourceError:
        case QSerialPort::WriteError:
        case QSerialPort::ReadError:
            debug("Micro scollegato?");
            break;

        default: {
            QString testo = QString ("SerialPortError %1").arg(serialPortError);
            debug(testo);
            break;
        }
    }
}


/*!
 * \brief SerialDev::fromDeviceSlot - Legge i byte dalla porta seriale, li decodifica
 *                                             e li passa a "dataFromDevice" per gestirli
 */
void SerialDev::fromDeviceSlot() {
    QByteArray buffer = readAll();
    int idx = 0;
    if (m_debug) {
        quint8 var;
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Rx(RAW) ";
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }
/*
    qDebug() << "SerialDev::fromDeviceSlot  start:" << idx <<
                "  buffer.length(): " << buffer.length() << "   ";
                */
    // Fin tanto che non sono arrivato al fondo del buffer, decodifico!
    while (idx < buffer.length()) {
        if (decodeMessage (buffer, m_bufferDest, idx, m_statoParser)) {
            if (m_debug) {
                quint8 var;
                QDebug debugBuffer = qDebug();
                debugBuffer << headDebug << "Rx ";
                foreach (var, m_bufferDest) {
                    debugBuffer << hex << var;
                }
            }

            if (m_ipChecked && m_bufferDest.length() >= 6) {
                qDebug() << "m_ipChecked:" << m_ipChecked;
                if (m_bufferDest.at(2) == ID_SCHEDA_IO) {
                    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
                        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost)) {
                            m_ipChecked = false;
                            m_ipAddress = buffer.at(5);
                            if (((quint8)buffer.at(5) >= 0x20) && ((quint8)buffer.at(5) <= 0x2F)) {
                                if ((address.toIPv4Address() & 0x000000FF) != (quint32)buffer.at(5)) {
                                    ricreaFileIp(buffer.at(5));
                                    #ifdef Q_WS_QWS
                                    system ("reboot");
                                    #endif
                                }
                            }
                        }
                    }
                }
            }

//            qDebug() << "m_bufferDest.length()" << m_bufferDest.length();
            if (m_powerOff && m_bufferDest.length() >= 3) {
                if (m_bufferDest.at(2) == POWER_OFF) {
                    m_powerOff = false;
                    qDebug() << "power off:";
                    QTimer::singleShot(1000, this, SLOT(powerOff()));
                }/* endif */
            }/* endif */

            emit dataFromDevice(m_bufferDest);
            // Ripulisco il buffer perche' gia' gestito
            m_bufferDest.clear();
        }
    }
}

void SerialDev::powerOff () {
#ifdef Q_WS_QWS
    //                    PowerManager::instance()->setOutput('0'); // spegne la scheda IO
    delete TcpServer::instance();
    system ("ifconfig wlan0 down");
    system ("poweroff");
#endif // #ifdef Q_WS_QWS
}

void SerialDev::bytesWrittenSlot(qint64) {
    start();
}

void SerialDev::ricreaFileIp(quint8 ip) {
    char buffer[128];
    sprintf (buffer, "cp -a startwlan%d.sh startwlan0.sh", ip);
    system (buffer);
}
