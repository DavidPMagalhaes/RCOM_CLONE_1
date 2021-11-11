#include "dataProtocol.h"

#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include "setFD.h"
#include "transmitter.h"
#include "receiver.h"

struct termios oldtio, newtio;
struct linkLayer link;

void initializeFrame(struct frame *frame)
{
    printf("initializing frame inside");
    fflush(stdout);
    char *newFrame = (char *)malloc(MAX_SIZE);
    printf("mem alloc");
    fflush(stdout);
    frame->frameSize = MAX_SIZE;
    frame->stuffedFrame = newFrame;
    frame->stuffedFrameSize = MAX_SIZE;
    printf("done alloc");
    fflush(stdout);
}

int llopen(int porta, linkType type)
{
    sprintf(link.port, "/dev/ttyS%d", porta);
    setFD(&link, &oldtio, &newtio);
    printf("set fd");
    fflush(stdout);
    initializeFrame(&link.frame);
    printf("initialing frame");
    fflush(stdout);

    link.type = type;
    switch (type)
    {
    case TRANSMITTER:
        printf("attempting to open transmitter");
        fflush(stdout);

        if (openTransmitter(&link) == 0)
        {
            return link.fd; //emissor.c
        }
        break;
    case RECEIVER:
        printf("attempting to open receiver");
        fflush(stdout);

        if (openReceiver(&link) > 0)
        {
            return link.fd; //recetor.c
        }
        break;
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
    int res;
    switch (link.type)
    {
    case TRANSMITTER:
        res = closeTransmitter(&link);
        break;
    case RECEIVER:
        res = closeReceiver(&link);
        break;
    }
    if (res != 0)
    {
        //What to do if res != 0?
        return res;
    }
    closeFD(fd, oldtio);
    return 0;
}