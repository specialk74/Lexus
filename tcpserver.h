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
protected slots:
    void newConnectionSlot();
    void erroSocketSlot(QAbstractSocket::SocketError);
protected:
    TcpServer (QObject * parent);
    void debug (const QString &testo);
    void deleteClient(QTcpSocket *socket);
private:
    static TcpServer *m_Instance;
    TcpClient* m_client;
    bool m_debug;
};

#endif // TCPSERVER_H
