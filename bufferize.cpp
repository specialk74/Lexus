#include <QDebug>
#include "bufferize.h"
#include "serialdev.h"

Bufferize *Bufferize::m_Instance = NULL;


Bufferize * Bufferize::instance (QObject *parent) {
    if (m_Instance == NULL) {
        new Bufferize(parent);
    }

    return m_Instance;
}

Bufferize::~Bufferize () {
    m_Instance = NULL;
}

Bufferize::Bufferize (QObject *parent) : QObject (parent) {
    qDebug() << __FILE__ << __LINE__ << __func__;

    m_Instance 			= this;
    m_idxAddSequence 	= 0;
    m_idxGetSequence 	= 0;
    m_idxAddInstantCmd 	= 0;
    m_idxGetInstantCmd 	= 0;
    m_dimVector			= 0;
}

void Bufferize::addBuffer(const QByteArray &buffer) {
    qDebug() << __FILE__ << __LINE__ << __func__;

    quint8 type = 0;
    if (m_dimVector == 0) {
        qFatal("Dim Vector is 0");
        return;
    }
#warning "Ricavare type message"
    type = buffer.at(0);
    switch (type) {
    case TYPE_SYNC:
        m_Sync = buffer;
        break;
    case TYPE_ISTANT_CMD:
        m_InstantCmd[m_idxAddInstantCmd] = buffer;
        m_idxAddInstantCmd++;
        m_idxAddInstantCmd %= m_dimVector;
        break;
    case TYPE_SEQUENCE:
        m_Sequence[m_idxAddSequence] = buffer;
        m_idxAddSequence++;
        m_idxAddSequence %= m_dimVector;
        break;
    default:
        qDebug() << "Type:"<< hex << type;
        qFatal("Message Type not Allowed:");
        break;
    }

    SerialDev::instance()->start();
}

bool Bufferize::getBuffer(QByteArray &buffer) {
    qDebug() << __FILE__ << __LINE__ << __func__;
    bool ret = false;
    if (m_dimVector == 0) {
        return false;
    }

    if (m_Sync.length() != 0) {
#warning "Controllare se fa la copia dei dati"
        buffer = m_Sync;
        m_Sync.clear();
        ret = true;
    }
    else if (m_idxAddInstantCmd != m_idxGetInstantCmd) {
        buffer = m_InstantCmd[m_idxGetInstantCmd];
        m_idxGetInstantCmd++;
        m_idxGetInstantCmd %= m_dimVector;
        ret = true;
    }
    else if (m_idxAddSequence != m_idxGetSequence) {
        buffer = m_Sequence[m_idxGetSequence];
        m_idxGetSequence++;
        m_idxGetSequence %= m_dimVector;
        ret = true;
    }

    return ret;
}

void Bufferize::setVector (const quint32 &dim) {
    qDebug() << __FILE__ << __LINE__ << __func__;
    m_dimVector = dim;
    m_Sequence.resize (dim);
    m_InstantCmd.resize (dim);
}
