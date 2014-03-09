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
    enum {
        FIRST_DLE = 0,
        FIRST_STX,
        TYPE_MESSAGE,
        DLE_STATE,
        FIRST_LENGTH,
        SECOND_LENGTH,
        LAST_DLE,
        LAST_ETX
    } m_state, m_rightState;

    static SerialDev * m_Instance;
    bool m_debug;
    QByteArray m_bufferData;
    QByteArray m_bufferTemp;
    quint16 m_len;
    quint16 m_idx;
    quint16 m_start;
    quint16 m_end;

//    bool m_sendInProgress;
};

#endif // SERIALDEV_H
