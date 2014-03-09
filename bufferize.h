#ifndef BUFFERIZE_H
#define BUFFERIZE_H

#include <QObject>
#include <QVector>

class Bufferize : public QObject {
    Q_OBJECT
public:
    static Bufferize * instance (QObject *parent = 0);
    ~Bufferize ();
    void addBuffer(const QByteArray &buffer);
    bool getBuffer(QByteArray &buffer);
    void setVector(const quint32 &dim);
    enum {
        TYPE_GET_ID = 0,
        TYPE_CMD_OUTPUT_DIGITAL = 0x20,
        TYPE_CMD_OUTPUT_ANALOG = 0x21,
        TYPE_CMD = 0x30,
        TYPE_SYNC = 0x31,
        TYPE_BUFFER_ALL_OUTPUT_DIGITAL = 0x40,
        TYPE_BUFFER_OUTPUT_DIGITAL = 0x41,
        TYPE_GET_DIP_SWITCH = 0x50
    };
protected:
    Bufferize(QObject *parent);
    void addSingleBuffer(const QByteArray &buffer);
private:
    static Bufferize *m_Instance;
    QVector<QByteArray> m_Sequence;
    QVector<QByteArray> m_InstantCmd;
    QByteArray m_Sync;

    quint32 m_idxAddSequence;
    quint32 m_idxGetSequence;
    quint32 m_idxAddInstantCmd;
    quint32 m_idxGetInstantCmd;

    quint32 m_dimVector;
};

#endif // BUFFERIZE_H
