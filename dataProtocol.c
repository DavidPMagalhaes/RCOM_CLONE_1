#include "dataProtocol.h"

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "setFD.h"
#include "transmitter.h"
#include "receiver.h"

struct termios oldtio, newtio;
struct linkLayer link;

int llopen(int porta, linkType type)
{
    sprintf(link.port, "/dev/ttyS%d", porta);
    setFD(&link, &oldtio, &newtio);
    link.type = type;
    link.sequenceNumber = 0;
    link.timeout = 3;
    link.numTransmissions = 3;
    switch (type)
    {
    case TRANSMITTER:
        if (openTransmitter(&link) == 0)
        {
            return link.fd;
        }
        break;
    case RECEIVER:
        if (openReceiver(&link) == 0)
        {
            return link.fd;
        }
        break;
    }
    return -1;
}

int llwrite(int fd, u_int8_t *buffer, int length)
{
    if (fd != link.fd)
    {
        printf("Wrong fd\n");
    }
    return writeTransmitter(&link, buffer, length);
}

int llread(int fd, u_int8_t *buffer)
{
    if (fd != link.fd)
    {
        printf("Wrong fd\n");
    }
    return readReceiver(&link, buffer);
}

int llclose(int fd)
{
    int res;
    switch (link.type)
    {
    case TRANSMITTER:
        res = closeTransmitter(&link);
        break;
    case RECEIVER:
        break;
    }
    closeFD(fd, oldtio);
    return res;
}