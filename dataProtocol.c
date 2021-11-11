#include <termios.h>

#include "dataProtocol.h"
#include "setFD.h"

struct termios oldtio, newtio;
struct linkLayer link;

void initializeFrame(struct frame *frame)
{
    char *newFrame = (char *)malloc(MAX_SIZE);
    frame->stuffedFrame = newFrame;
    frame->stuffedFrameSize = MAX_SIZE;
}

int llopen(int porta, linkType type)
{
    sprintf(link.port, "/dev/ttyS%d", porta);
    setFD(&link, &oldtio, &newtio);
    initializeFrame(&link.frame);
    switch (type)
    {
    case TRANSMITTER:
        if (openTransmitter(link) == 0)
        {
            return link.fd; //emissor.c
        }
    case RECEIVER:
    int fd;
        if (receiverSomething(fd) > 0)
        {
            return fd; //recetor.c
        }
    }
    return -1;
}

int llwrite(int fd, char *buffer, int length)
{
}

int llread(int fd, char *buffer)
{
}

int llclose(int fd)
{

    return -1;
}
