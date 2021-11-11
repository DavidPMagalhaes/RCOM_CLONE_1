#ifndef DATAPROTOCOL_H
#define DATAPROTOCOL_H

#define MAX_SIZE 255

typedef enum
{
    RECEIVER,
    TRANSMITTER
} linkType;

struct frame
{
    char frame[MAX_SIZE]; /*Trama*/
    int frameSize;
    char *stuffedFrame;
    int stuffedFrameSize;
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

void initializeFrame(struct frame *frame);

int llopen(int porta, linkType type);

int llwrite(int fd, char *buffer, int length);

int llread(int fd, char *buffer);

int llclose(int fd);

#endif