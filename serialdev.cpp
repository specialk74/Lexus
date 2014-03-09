#include <QDebug>
#include "bufferize.h"
#include "serialdev.h"

#define DLE (0x10)
#define STX (0x02)
#define ETX (0x03)

static const char headDebug[] = "[SlaveSerialDev]";
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
    m_idx = 0;
    m_state = FIRST_DLE;
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
    int len = buffer.length();
    if (m_debug) {
        quint8 var;
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Rx(RAW) ";
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }

    quint16 increment;

    for (quint16 idx = 0; idx < len; idx++) {
        m_bufferData.append(m_bufferTemp.at(m_idx));
        switch (m_state) {
        case FIRST_DLE:
            increment = 1;
            if (m_bufferTemp.at(m_idx) == DLE) {
                m_state = FIRST_STX;
                m_start = m_idx;
                m_bufferData.clear();
                m_bufferData.append(DLE);
            }
            break;
        case FIRST_STX:
            m_state = (m_bufferTemp.at(m_idx) == STX) ? TYPE_MESSAGE : FIRST_DLE;
            break;
        case TYPE_MESSAGE:
            if (m_bufferTemp.at(m_idx) == DLE) {
                m_state = DLE_STATE;
                m_rightState = FIRST_LENGTH;
            }
            else {
                m_state = FIRST_LENGTH;
            }
            break;
        case DLE_STATE:
            m_state = (m_bufferTemp.at(m_idx) == DLE) ? m_rightState : FIRST_DLE;
            break;
        case FIRST_LENGTH:
           m_len = m_bufferTemp.at(m_idx);
           if (m_bufferTemp.at(m_idx) == DLE) {
               m_state = DLE_STATE;
               m_rightState = SECOND_LENGTH;
           }
           else {
               m_state = SECOND_LENGTH;
           }
           break;
        case SECOND_LENGTH:
            m_len += m_bufferTemp.at(m_idx);
            m_end = m_start + m_len - 1;
           if (m_bufferTemp.at(m_idx) == DLE) {
               m_state = DLE_STATE;
               m_rightState = LAST_DLE;
           }
           else {
               m_state = LAST_DLE;
           }
           break;
        case LAST_DLE:
            if (m_idx >= m_end) {
                m_state = (m_bufferTemp.at(m_idx) == DLE) ? LAST_ETX : FIRST_DLE;
            }
            break;
        case LAST_ETX:
            m_state = FIRST_DLE;
            if (m_bufferTemp.at(m_idx) == ETX) {
                if (m_debug) {
                    quint8 var;
                    QDebug debugBuffer = qDebug();
                    debugBuffer << headDebug << "Rx ";
                    foreach (var, m_bufferData) {
                        debugBuffer << hex << var;
                    }
                }
                emit dataFromDevice(m_bufferData);
                m_bufferData.clear();
                if (m_idx >= m_bufferTemp.length() - 1) {
                    m_idx = 0;
                    m_bufferTemp.clear();
                    increment = 0;
                }
            }
            break;
        }
        m_idx += increment;
    }
}

void SerialDev::bytesWritten(qint64) {
    start();
}
