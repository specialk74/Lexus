#ifndef BUFFERIZE_H
#define BUFFERIZE_H

#include <QObject>
#include <QVector>
#include "utils.h"

class TcpClient;

class Bufferize : public QObject {
    Q_OBJECT
public:
    static Bufferize * instance (QObject *parent = 0);
    ~Bufferize ();
    void addBuffer(const QByteArray &buffer, TcpClient *client = NULL);
    bool getBuffer(QByteArray &buffer);
    void setVector(const quint32 &dim);
    enum {
        TYPE_GET_ID = 0,
        TYPE_ID_MODULO = 0x01,
        TYPE_ID_SCHEDA_IO = 0x02,
        TYPE_CMD_OUTPUT_DIGITAL = 0x20,
        TYPE_CMD_OUTPUT_ANALOG = 0x21,
        TYPE_CMD_POWER_OFF = 0x2F,
        TYPE_SYNC = 0x31,
        TYPE_BUFFER_ALL_OUTPUT_DIGITAL = 0x40,
        TYPE_BUFFER_OUTPUT_DIGITAL = 0x41,
        TYPE_BUFFER_ALL_OUTPUT_ANALOG = 0x42,
        TYPE_GET_OUTPUT_DIGITAL = 0x50,
        TYPE_OUTPUT_DIGITAL = 0x51,
        TYPE_GET_OUTPUT_ANALOG = 0x52,
        TYPE_OUTPUT_ANALOG = 0x53,
        TYPE_GET_INPUT_DIGITAL = 0x60,
        TYPE_INPUT_DIGITAL = 0x61,
    };
protected:
    Bufferize(QObject *parent);
    void addSingleBuffer(const QByteArray &buffer, TcpClient *client);
private:
    static Bufferize *m_Instance;
    QVector<QByteArray> m_Sequence;
    QVector<QByteArray> m_InstantCmd;
    QByteArray m_Sync;
    QByteArray m_bufferDest;
    STATO_DECODER_MSG m_statoParser;

    quint32 m_idxAddSequence;
    quint32 m_idxGetSequence;
    quint32 m_idxAddInstantCmd;
    quint32 m_idxGetInstantCmd;

    quint32 m_dimVector;
};

#endif // BUFFERIZE_H
