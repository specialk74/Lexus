#ifndef SERIALDEV_H
#define SERIALDEV_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "utils.h"

class Bufferize;

class SerialDev : public QSerialPort
{
    Q_OBJECT
public:
    static SerialDev *instance(QObject *parent = 0);
    ~SerialDev();
    void setDebug (const bool &val);
    void start ();
    bool configPort(const QString &name);
signals:
    void dataFromDevice(const QByteArray &buffer);
protected slots:
    void bytesWrittenSlot(qint64 bytes);
    void fromDeviceSlot();
    void errorSlot(QSerialPort::SerialPortError);
protected:
    SerialDev(QObject *parent);
    void sendMsg (const QByteArray &buffer);
    void debug (const QString &testo);
    void ricreaFileIp(quint8 ip);
private:
    static  SerialDev           *m_Instance;
            STATO_DECODER_MSG   m_statoParser;
            bool                m_debug;
            QByteArray          m_bufferDest;
            bool                m_ipChecked;
};

#endif // SERIALDEV_H
