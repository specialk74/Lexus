#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include "powermanager.h"

/*
#define BANCATA (3)
#define NUMERO_GPIO (21)
#define MAX_BUF (128)
#define GPIO_POWEROFF (32*BANCATA+NUMERO_GPIO)
*/
#define TIMEOUT (8)

const char unexportGpio[] = "/sys/class/gpio/unexport";
const char exportGpio[] = "/sys/class/gpio/export";
const char gpioGpio[] = "/sys/class/gpio/gpio";

PowerManager *PowerManager::m_Instance = NULL;

PowerManager *PowerManager::instance (QObject *parent)
{
    if (m_Instance == NULL) {
        new PowerManager(parent);
    }
    return m_Instance;
}

PowerManager::PowerManager(QObject *parent):QObject(parent)
{
    m_counter = TIMEOUT;
    m_Instance = this;
}

void PowerManager::setIO (quint16 input, quint16 output) {
    m_input = input;
    m_output = output;

    QString nomeFile;

    if (m_output) {
        getNFValue(nomeFile, m_output);
        if (QFile::exists(nomeFile) == true) {
            inport_exportGPIO(unexportGpio, m_output);
        }


        if (inport_exportGPIO (exportGpio, m_output) && setDirection (m_output, DIR_OUT)) {
            setOutput ('1'); // alimenta la scheda IO
        }
    }

    if (m_input) {
        getNFValue(nomeFile, m_input);
        if (QFile::exists(nomeFile) == true) {
            inport_exportGPIO(unexportGpio, m_input);
        }
        if (inport_exportGPIO (exportGpio, m_input) && setDirection (m_input, DIR_IN)) {
            getNFValue(m_nomeFile, m_input);
            connect(&m_timer, SIGNAL(timeout()), this, SLOT(timeoutSlot()));
            m_timer.start(500);
        }
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

#ifdef Q_WS_QWS
    if (m_counter == 0) {
        setOutput('0');
        system ("poweroff"); // urbino
    }/* endif */
#endif // #ifdef Q_WS_QWS
}

void PowerManager::setOutput (const char valore) {
    QString nomeFileOutput;
    getNFValue(nomeFileOutput, m_output);
    QFile outputFile (nomeFileOutput);
    if (outputFile.open(QIODevice::WriteOnly) == true) {
        outputFile.write(&valore, 1);
        outputFile.close();
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

bool PowerManager::setDirection (quint16 gpio, Direction dir) {
    bool ret = false;
    QString nomeFile;
    getNFDirection(nomeFile, gpio);
    QFile fileToWrite (nomeFile);
    if (fileToWrite.open(QIODevice::WriteOnly) == true) {
        QString buf;
        QTextStream out(&fileToWrite);
        if (dir == DIR_IN) {
            out << "in";
        }
        else if (dir == DIR_OUT) {
            out << "out";
        }
        fileToWrite.close();
        ret = true;
    }
    else {
        qDebug() << "Unable to open" << nomeFile;
    }

    return ret;
}

