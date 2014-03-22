#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>
#include <QTimer>

class PowerManager : public QObject
{
    Q_OBJECT
public:
    explicit PowerManager(QObject *parent = 0);        
protected slots:
    void timeoutSlot();
protected:
    bool inport_exportGPIO (const QString &inport_export, quint16 gpio);
    bool setDirection (quint16 gpio);
    void getNFDirection (QString &nomeFile, quint16 gpio);
    void getNFValue (QString &nomeFile, quint16 gpio);
private:
    QTimer m_timer;
    quint8 m_counter;
    QString m_nomeFile;
};

#endif // POWERMANAGER_H
