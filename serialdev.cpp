#include <QDebug>
#include "bufferize.h"
#include "serialdev.h"

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
    qDebug() << __FILE__ << __LINE__ << __func__;
    m_Instance = this;
    m_debug = false;
//    m_sendInProgress = false;

    qDebug() << headDebug << "CTor";
}

SerialDev::~SerialDev()
{
    qDebug() << __FILE__ << __LINE__ << __func__;
    m_Instance = NULL;
    qDebug() << headDebug << "DTor" << portName();
}

void SerialDev::setDebug (const bool &val)
{
    qDebug() << __FILE__ << __LINE__ << __func__;
    m_debug = val;
}

void SerialDev::debug (const QString &testo)
{
    qDebug() << __FILE__ << __LINE__ << __func__;
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
    qDebug() << __FILE__ << __LINE__ << __func__;
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

   return true;
}

void SerialDev::sendMsg (const QByteArray &buffer) {
//    m_sendInProgress = true;
    qDebug() << __FILE__ << __LINE__ << __func__;
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
    qDebug() << __FILE__ << __LINE__ << __func__;
    QByteArray buffer;
    #warning "Controllare se funziona bytesToWrite"
//	if ((m_sendInProgress == false) && m_bufferize && m_bufferize->getBuffer(buffer)) {
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
    qDebug() << __FILE__ << __LINE__ << __func__;
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
#warning "Collegare al client"
}

void SerialDev::bytesWritten(qint64) {
//    m_sendInProgress = false;
    start();
}
