#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include "bufferize.h"
#include "serialdev.h"
#include "tcpserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    quint16 port = 0;
    bool debug = false;
    QRegExp rxArgPort("-p([0-9]{1,})");
    QRegExp rxArgDevice("-d(\\S+)");
    QRegExp rxArgDebug("-g");
    QString device = QString ("/dev/ttyUSB0");

    for (int i = 1; i < args.size(); ++i) {
        qDebug() << args.at(i);
        if ((rxArgPort.indexIn(args.at(i)) != -1 )) {
            port =  rxArgPort.cap(1).toInt();
            qDebug() << "port" << port;
        }
        else if (rxArgDebug.indexIn(args.at(i)) != -1) {
            debug = true;
        }
        else if (rxArgDevice.indexIn(args.at(i)) != -1) {
            //device = rxArgPort.cap(1);
            qDebug() << "rxArgPort.cap(1)" << rxArgPort.cap(1);
        }
    }

    Bufferize::instance()->setVector(0xFFF);
/*
    if (device.isEmpty()) {
        qDebug() << "Need device";
        return -1;
    }
    */
    SerialDev::instance()->setDebug(debug);
    bool ret = SerialDev::instance()->configPort(device);
    if (ret == false) {
        return -1;
    }

    if (port == 0) {
        qDebug() << "Need PORT number";
        return -1;
    }
    TcpServer::instance()->setDebug(debug);
    ret = TcpServer::instance()->listen (QHostAddress::Any, port);
    if (ret == false) {
        qDebug() << TcpServer::instance()->errorString();
    }

    return app.exec();
}
