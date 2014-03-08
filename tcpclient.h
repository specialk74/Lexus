#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
class QTcpSocket;

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);
    void setSocket (QTcpSocket *socket);
    QTcpSocket * getSocket ();
    void setDebug (const bool &val) { m_debug = val; }
    ~TcpClient();
protected slots:
    void fromClientsSlot();
    void send (const QByteArray &buffer);
protected:
    void debug (const QString &testo);
    void ricreaFileIp(quint8);
    QByteArray m_buffer;
    QTcpSocket *m_socket;
    bool m_debug;
    bool m_ipChecked;
};


#endif // TCPCLIENT_H
