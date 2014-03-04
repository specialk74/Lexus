#ifndef SERIALDEV_H
#define SERIALDEV_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

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
    void bytesWritten(qint64 bytes);
    void fromDeviceSlot();
    void errorSlot(QSerialPort::SerialPortError);
protected:
    SerialDev(QObject *parent);
    void sendMsg (const QByteArray &buffer);
    void debug (const QString &testo);
private:
    static SerialDev * m_Instance;
    bool m_debug;
//    bool m_sendInProgress;
};

#endif // SERIALDEV_H
