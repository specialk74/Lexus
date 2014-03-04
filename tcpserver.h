#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

class TcpClient;

class TcpServer : public QTcpServer {
    Q_OBJECT
public:
    static TcpServer * instance (QObject * parent = 0);
    ~TcpServer();
    void setDebug (const bool &val);
protected:
    TcpServer (QObject * parent);
    void debug (const QString &testo);
    void deleteClient(QTcpSocket *socket);
    void newConnectionSlot();
    void erroSocketSlot(QAbstractSocket::SocketError);
private:
    static TcpServer *m_Instance;
    TcpClient* m_client;
    bool m_debug;
};

#endif // TCPSERVER_H
