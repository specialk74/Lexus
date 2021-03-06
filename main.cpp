#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include "bufferize.h"
#include "serialdev.h"
#include "tcpserver.h"
#include "udpserver.h"
#include "powermanager.h"

void usage (char *nomeExe) {
    qDebug() << nomeExe << "-t[Tcp Port] -u[Udp Port] -d[Serial Device] -g: Enable Debug";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();

    quint16 tcpPort = 0;
    quint16 udpPort = 0;
    quint16 inputPort = 0;
    quint16 outputPort = 0;
    bool debug = false;
    QRegExp rxArgTcpPort("-t([0-9]{1,})");
    QRegExp rxArgUdpPort("-u([0-9]{1,})");
    QRegExp rxArgInput("-i([0-9]{1,})");
    QRegExp rxArgOutput("-o([0-9]{1,})");
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
        else if ((rxArgInput.indexIn(args.at(i)) != -1 )) {
            inputPort =  rxArgInput.cap(1).toInt();
            qDebug() << "Input port" << inputPort;
        }
        else if ((rxArgOutput.indexIn(args.at(i)) != -1 )) {
            outputPort =  rxArgOutput.cap(1).toInt();
            qDebug() << "Output port" << outputPort;
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

    UdpServer::instance()->setDebug(debug);
    ret = UdpServer::instance()->bind(udpPort);
    if (ret == false) {
        qDebug() << UdpServer::instance()->errorString();
        return -1;
    }

#ifdef Q_WS_QWS
    PowerManager powermanager;
    powermanager.setIO(inputPort, outputPort);
#endif

    return app.exec();
}
