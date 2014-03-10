#include <QDebug>
#include "bufferize.h"
#include "serialdev.h"

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
    m_Instance      = this;
    m_debug         = false;
    m_statoParser   = STATO_DLE_STX;

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

            emit dataFromDevice(m_bufferDest);
            // Ripulisco il buffer perche' gia' gestito
            m_bufferDest.clear();
        }
    }
}

void SerialDev::bytesWrittenSlot(qint64) {
    start();
}
