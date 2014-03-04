#include <QCoreApplication>
#include "bufferize.h"
#include "serialdev.h"
#include "tcpserver.h"

#define SERVER_PORT (7000)

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    
    SerialDev::instance()->setDebug(true);
    bool ret = SerialDev::instance()->configPort("/dev/ttyUSB1");
    qDebug() << "ret: " << ret;

    TcpServer::instance()->setDebug(true);
    ret = TcpServer::instance()->listen (QHostAddress::Any, SERVER_PORT);
    qDebug() << "ret: " << ret << TcpServer::instance()->errorString();
    Bufferize::instance();

    return a.exec();
}
