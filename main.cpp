#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include "bufferize.h"
#include "serialdev.h"
#include "tcpserver.h"
#include "udpserver.h"

void usage (char *nomeExe) {
    qDebug() << nomeExe << "-t[Tcp Port]" << "-u[Udp Port]" << "-d[Serial Device]" << "-g: Enable Debug";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    quint16 tcpPort = 0;
    quint16 udpPort = 0;
    bool debug = false;
    QRegExp rxArgTcpPort("-t([0-9]{1,})");
    QRegExp rxArgUdpPort("-u([0-9]{1,})");
    QRegExp rxArgDevice("-d(\\S+)");
    QRegExp rxArgDebug("-g");
    QString device = QString ("/dev/ttyUSB0");

    for (int i = 1; i < args.size(); ++i) {
        if ((rxArgTcpPort.indexIn(args.at(i)) != -1 )) {
            tcpPort =  rxArgTcpPort.cap(1).toInt();
            qDebug() << "Tcp port" << tcpPort;
        }
        else if (rxArgDebug.indexIn(args.at(i)) != -1) {
            debug = true;
        }
        else if (rxArgDevice.indexIn(args.at(i)) != -1) {
            device = rxArgDevice.cap(1);
            qDebug() << "Device" << device;
        }
        else if ((rxArgUdpPort.indexIn(args.at(i)) != -1 )) {
            udpPort =  rxArgUdpPort.cap(1).toInt();
            qDebug() << "Udp port" << udpPort;
        }
        else {
            qDebug() << "Parametro non riconosciuto: " << args.at(i);
        }
    }

    Bufferize::instance()->setVector(0xFFF);

    if (device.isEmpty()) {
        qDebug() << "Need device";
        usage(argv[0]);
        return -1;
    }

    SerialDev::instance()->setDebug(debug);
    bool ret = SerialDev::instance()->configPort(device);
    if (ret == false) {
        return -1;
    }

    if (tcpPort == 0) {
        qDebug() << "Need Tcp Port number";
        usage(argv[0]);
        return -1;
    }
    TcpServer::instance()->setDebug(debug);
    ret = TcpServer::instance()->listen (QHostAddress::Any, tcpPort);
    if (ret == false) {
        qDebug() << TcpServer::instance()->errorString();
    }

    if (udpPort == 0) {
        qDebug() << "Need Udp Port number";
        usage(argv[0]);
        return -1;
    }

    ret = UdpServer::instance()->bind(QHostAddress::LocalHost, udpPort);
    if (ret == false) {
        qDebug() << UdpServer::instance()->errorString();
    }

    return app.exec();
}
