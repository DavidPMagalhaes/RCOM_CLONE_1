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

// void initializeFrame(struct frame *frame)
// {
//     printf("initializing frame inside");
//     fflush(stdout);
//     u_int8_t *newFrame = (u_int8_t *)malloc(MAX_SIZE);
//     printf("mem alloc");
//     fflush(stdout);
//     frame->frameSize = MAX_SIZE;
//     frame->stuffedFrame = newFrame;
//     frame->stuffedFrameSize = MAX_SIZE;
//     frame->stuffedFrameUsedSize = 0;
//     printf("done alloc");
//     fflush(stdout);
// }

int llopen(int porta, linkType type)
{
    sprintf(link.port, "/dev/ttyS%d", porta);
    setFD(&link, &oldtio, &newtio);
    printf("set fd");
    fflush(stdout);
    // initializeFrame(&link.frame);
    printf("initialing frame");
    fflush(stdout);

    link.type = type;
    link.sequenceNumber = 0;
    link.timeout = 3;
    link.numTransmissions =3;
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

        if (openReceiver(&link) == 0)
        {
            return link.fd; //recetor.c
        }
        break;
    }
    return -1;
}

int llwrite(int fd, u_int8_t *buffer, int length)
{
    if(fd != link.fd){
        // do something
    }
    return writeTransmitter(&link, buffer, length);
}

int llread(int fd, u_int8_t *buffer)
{
    if(fd != link.fd){
        // do something
    }
    return readReceiver(&link, buffer);
}

int llclose(int fd)
{
    int res;
    switch (link.type)
    {
    case TRANSMITTER:
    printf("closing transmitter\n");
        res = closeTransmitter(&link);
        break;
    case RECEIVER:
        // We don't do anything and just close the fd
        // res = closeReceiver(&link);
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