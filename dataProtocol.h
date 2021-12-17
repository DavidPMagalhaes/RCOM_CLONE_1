#ifndef DATAPROTOCOL_H
#define DATAPROTOCOL_H

#include <sys/types.h>

// Change according to frame size. Change application.h define was well
// #define MAX_SIZE 64
// #define MAX_SIZE 128
#define MAX_SIZE 255
// #define MAX_SIZE 512
// #define MAX_SIZE 1024
#define MAX_BUFFER_SIZE MAX_SIZE * 2 + 5 + 2

typedef enum
{
    RECEIVER,
    TRANSMITTER
} linkType;

struct frame
{
    u_int8_t frame[MAX_SIZE * 2 + 5 + 2]; // Frame maximum size
    // *2 due to the stsuffing. 5 for the commands, 2 for the date proction byte that might need stuffing as well
    int frameUsedSize;
};

struct linkLayer
{
    char port[20]; // Device /dev/ttySx
    int fd;
    int baudRate;                  // Transmission speed
    unsigned int sequenceNumber;   // Frame sequence number: 0, 1
    unsigned int timeout;          // Timeout s
    unsigned int numTransmissions; // Retries on timeout
    struct frame frame;
    linkType type;
};

int llopen(int porta, linkType type);

int llwrite(int fd, u_int8_t *buffer, int length);

int llread(int fd, u_int8_t *buffer);

int llclose(int fd);

#endif //DATAPROTOCOL_H