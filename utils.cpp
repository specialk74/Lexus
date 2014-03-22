#include <stdio.h>
#include "utils.h"

bool decodeMessage (const QByteArray &bufferIn, QByteArray &bufferOut, int & idx, STATO_DECODER_MSG & stato)
{
    int end = bufferIn.length();
    char dato;
//printf ("dentro decodeMessage idx: %d end: %d\n", idx, end);
    for (;idx < end; idx++)
    {
        dato = bufferIn[idx];
        bufferOut.append(dato);

//    printf (" - dato:%x - stato:%x  idx:%d\n", dato, stato, idx);
        switch (stato)
        {
        // Cerco il primo DLE della sequenza DLE-STX
        case STATO_DLE_STX:
            if (dato == DLE) {
                bufferOut.clear();
                bufferOut.append(DLE);
                stato = STATO_STX;
            }
            break;
        // Cerco l'STX subito dopo il DLE iniziale
        case STATO_STX:
            stato = (dato == STX) ? STATO_DATO : STATO_DLE_STX;
            break;
        // Sono nella parte dati
        case STATO_DATO:
            if (dato == DLE) {
                stato = STATO_ETX; // Se trovo un DLE controllo il carattere che viene dopo
            }
            break;
        // Arrivo dallo stato precendente
        case STATO_ETX:
            if (dato == DLE) {
                // Ho trovato un altro DLE: lo inserisco nel buffer e mi riporto allo stato
                // precedente
                stato = STATO_DATO;
            }
            else if (dato == ETX) {
                stato = STATO_DLE_STX;
                idx++;
                return true;
            }
            else {
                // Errore: questo carattere non doveva essere qua. Ripulisco il buffer
                // e ricomincio dall'inizio
                stato = STATO_DLE_STX;
            }
            break;
        }
    }

    // Non ho trovato un buffer valido completo
//printf ("\n");
    return false;
}
