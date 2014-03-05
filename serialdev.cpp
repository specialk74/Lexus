#include <QDebug>
#include "bufferize.h"
#include "serialdev.h"

#define DLE (0x10)
#define STX (0x02)
#define ETX (0x03)

static const char headDebug[] = "[SerialDev]";
SerialDev * SerialDev::m_Instance = NULL;

SerialDev *SerialDev::instance(QObject *parent) {
    if (m_Instance == NULL) {
        new SerialDev(parent);
    }

    return m_Instance;
}

SerialDev::SerialDev(QObject *parent) :
    QSerialPort(parent)
{
    m_Instance = this;
    m_debug = false;
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
    QByteArray bufferOut;
    quint8 var;

    bufferOut.append(DLE);
    bufferOut.append(STX);

    foreach (var, buffer) {
        if (var == DLE) {
            bufferOut.append(DLE);
        }
        bufferOut.append(var);
    }
    bufferOut.append(DLE);
    bufferOut.append(ETX);

    if (m_debug) {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Tx ";
        foreach (var, bufferOut) {
            debugBuffer << hex << var;
        }
    }
    write(bufferOut);
    flush();
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
 * \brief Rs232DevicePrivate::fromDeviceSlot - Legge i byte dalla porta seriale, li decodifica
 *                                             e li passa a "handleMsgRxFromDevice" per gestirli
 */
void SerialDev::fromDeviceSlot() {
    QByteArray buffer = readAll();
    m_bufferTemp.append(buffer);
    int len = m_bufferTemp.length();
    if (    (m_bufferTemp.at(0) == DLE) && (m_bufferTemp.at(1) == STX) &&
            (m_bufferTemp.at(len - 2) == DLE) && (m_bufferTemp.at(len - 1) == ETX) ) {
        quint8 var;
        bool foundDLE = false;
        if (m_debug) {
            QDebug debugBuffer = qDebug();
            debugBuffer << headDebug << "Rx ";
            foreach (var, m_bufferTemp) {
                debugBuffer << hex << var;
            }
        }

        m_bufferTemp.remove(len - 2, 2);
        m_bufferTemp.remove(0, 2);
        m_bufferData.clear();

        foreach (var, m_bufferTemp) {
            if (var == DLE) {
                if (foundDLE == false) {
                    m_bufferData.append(var);
                }
                foundDLE = !foundDLE;
            }
            else {
                m_bufferData.append(var);
            }
        }

        emit dataFromDevice(m_bufferData);

        m_bufferTemp.clear();
    }
}

void SerialDev::bytesWritten(qint64) {
    start();
}
