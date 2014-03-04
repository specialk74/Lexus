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
        TYPE_NONE = 0,
        TYPE_SYNC,
        TYPE_ISTANT_CMD,
        TYPE_SEQUENCE
    };
protected:
    Bufferize(QObject *parent);
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
