#ifndef DATAPROTOCOL_H
#define DATAPROTOCOL_H

#include <sys/types.h>
#include "options.h"
#include "physicalProtocol.h"

struct PHYSICAL_OPTIONS;

typedef enum
{
    RECEIVER,
    TRANSMITTER
} linkType;

struct frame
{
    u_int8_t frame[MAX_BUFFER_SIZE]; // Trama
    // MAX_SIZE is maximum data size. *2 due to the stsuffing. 5 for the commands, 2 for the date proction byte that might need stuffing as well
    int frameUsedSize;
};

struct linkLayer
{
    char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
    int fd;
    int baudRate;                  /*Velocidade de transmissão*/
    unsigned int sequenceNumber;   /*Número de sequência da trama: 0, 1*/
    unsigned int timeout;          /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso de
falha*/
    struct frame frame;
    linkType type;
};

int llconfigure(struct PHYSICAL_OPTIONS options);

int llopen(int porta, linkType type);

int llwrite(int fd, u_int8_t *buffer, int length);

int llread(int fd, u_int8_t *buffer);

int llclose(int fd);

#endif