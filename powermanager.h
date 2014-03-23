#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>
#include <QTimer>

class PowerManager : public QObject
{
    Q_OBJECT
public:
    explicit PowerManager(QObject *parent = 0);
    void setIO (quint16 input, quint16 output);
protected slots:
    void timeoutSlot();
protected:
    enum Direction {
        DIR_IN,
        DIR_OUT
    };
    bool inport_exportGPIO (const QString &inport_export, quint16 gpio);
    bool setDirection (quint16 gpio, Direction dir);
    void getNFDirection (QString &nomeFile, quint16 gpio);
    void getNFValue (QString &nomeFile, quint16 gpio);
    void setOutput (const char valore);
private:
    QTimer m_timer;
    quint8 m_counter;
    QString m_nomeFile;
    quint16 m_input;
    quint16 m_output;
};

#endif // POWERMANAGER_H
