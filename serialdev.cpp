#include <QDebug>
#include "bufferize.h"
#include "serialdev.h"

static const char headDebug[] = "[SerialDev]";

SerialDev::SerialDev(const QSerialPortInfo &info, QObject *parent) :
    QSerialPort(info, parent)
{
    m_debug = false;
    m_sendInProgress = false;

    qDebug() << headDebug << "CTor" << info.portName();

    // Sono riuscito a configurare la porta?
    if (configPort())
    {
        connect(this, SIGNAL(error(QSerialPort::SerialPortError)),
                this, SLOT(errorSlot(QSerialPort::SerialPortError)));
        connect(this, SIGNAL(readyRead()), this, SLOT(fromDeviceSlot()));
    }
}

SerialDev::~SerialDev()
{
    qDebug() << headDebug << "DTor" << portName();
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
bool SerialDev::configPort ()
{
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

   return true;
}

void SerialDev::sendMsg (const QByteArray &buffer) {
    m_sendInProgress = true;
    if (m_debug) {
        QDebug debugBuffer = qDebug();
        debugBuffer << headDebug << "Tx ";
        quint8 var;
        foreach (var, buffer) {
            debugBuffer << hex << var;
        }
    }

    write(buffer);
    flush();
}

void SerialDev::start () {
    QByteArray buffer;
    #warning "Controllare se funziona bytesToWrite"
//	if ((m_sendInProgress == false) && m_bufferize && m_bufferize->getBuffer(buffer)) {
    if ((bytesToWrite() == 0) && m_bufferize && m_bufferize->getBuffer(buffer)) {
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
}

void SerialDev::bytesWritten(qint64 bytes) {
    m_sendInProgress = false;
    start();
}
