#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "powermanager.h"

#define BANCATA (3)
#define NUMERO_GPIO (19)
#define MAX_BUF (128)
#define GPIO_POWEROFF (32*BANCATA+NUMERO_GPIO)
#define TIMEOUT (4)

const char unexportGpio[] = "/sys/class/gpio/unexport";
const char exportGpio[] = "/sys/class/gpio/export";
const char gpioGpio[] = "/sys/class/gpio/gpio";

PowerManager::PowerManager(QObject *parent) :
    QObject(parent) {
    m_counter = TIMEOUT;

    QString nomeFile;
    getNFValue(nomeFile, GPIO_POWEROFF);
    if (QFile::exists(nomeFile) == true) {
        inport_exportGPIO(unexportGpio, GPIO_POWEROFF);
    }

    if (inport_exportGPIO (exportGpio, GPIO_POWEROFF) && setDirection (GPIO_POWEROFF)) {
        getNFValue(m_nomeFile, GPIO_POWEROFF);
        connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
        m_timer.start(1000);
    }
}


void PowerManager::timeoutSlot() {
    QFile inputFile (m_nomeFile);
    if (inputFile.open(QIODevice::ReadOnly) == true) {
        QByteArray buffer = inputFile.readAll();
        if (buffer.at(0) == '0') {
            m_counter--;
            qDebug() << "Poweroff countdown" << m_counter;
        }
        else {
            m_counter = TIMEOUT;
        }
        inputFile.close();
    }
    else {
        m_counter = TIMEOUT;
    }

    if (m_counter == 0) {
        system ("poweroff");
    }
}

void PowerManager::getNFDirection (QString &nomeFile, quint16 gpio) {
    nomeFile = QString ("%1%2/direction").arg(gpioGpio).arg(gpio);
}

void PowerManager::getNFValue (QString &nomeFile, quint16 gpio) {
    nomeFile = QString ("%1%2/value").arg(gpioGpio).arg(gpio);
}

bool PowerManager::inport_exportGPIO (const QString &nomeFile, quint16 gpio) {
    bool ret = false;
    QFile fileToWrite (nomeFile);
    if (fileToWrite.open(QIODevice::WriteOnly) == true) {
        QString buf;
        QTextStream out(&fileToWrite);
        buf.sprintf("%d", gpio);
        out << buf;
        fileToWrite.close();
        ret = true;
    }
    else {
        qDebug() << "Unable to open" << nomeFile;
    }

    return ret;
}

bool PowerManager::setDirection (quint16 gpio) {
    bool ret = false;
    QString nomeFile;
    getNFDirection(nomeFile, gpio);
    QFile fileToWrite (nomeFile);
    if (fileToWrite.open(QIODevice::WriteOnly) == true) {
        QString buf;
        QTextStream out(&fileToWrite);
        out << "in";
        fileToWrite.close();
        ret = true;
    }
    else {
        qDebug() << "Unable to open" << nomeFile;
    }

    return ret;
}

