#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>

#define DLE (0x10)
#define STX (0x02)
#define ETX (0x03)

enum STATO_DECODER_MSG {
    STATO_DLE_STX,
    STATO_STX,
    STATO_DATO,
    STATO_ETX
};

bool decodeMessage (const QByteArray &bufferIn, QByteArray &bufferOut, int & idx, STATO_DECODER_MSG & stato);


#endif // UTILS_H
