#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QUdpSocket>

class UdpServer : public QUdpSocket
{
    Q_OBJECT
public:
    static UdpServer *instance(QObject *parent = 0);
    ~UdpServer();
    void setDebug (const bool &val) { m_debug = val; }
protected slots:
    void readPendingDatagrams ();
protected:
    explicit UdpServer(QObject *parent = 0);
private:
    static UdpServer *m_Instance;
    bool    m_debug;
};

#endif // UDPSERVER_H
