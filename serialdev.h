#ifndef SERIALDEV_H
#define SERIALDEV_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class Bufferize;

class SerialDev : public QSerialPort
{
    Q_OBJECT
public:
    explicit SerialDev(const QSerialPortInfo &info, QObject *parent);
    ~SerialDev();
    void setDebug (const bool &val);
    void start ();
protected slots:
    void bytesWritten(qint64 bytes);
    void fromDeviceSlot();
    void errorSlot(QSerialPort::SerialPortError);
protected:
    bool configPort();
    void sendMsg (const QByteArray &buffer);
    void debug (const QString &testo);
private:
    bool m_debug;
    bool m_sendInProgress;
    Bufferize *m_bufferize;
};

#endif // SERIALDEV_H
