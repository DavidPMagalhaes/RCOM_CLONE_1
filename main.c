#include "dataProtocol.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{

    linkType receiver;
    linkType transmitter;
    receiver = RECEIVER;
    transmitter = TRANSMITTER;
    int fd_receiver, fd_transmitter;
    int flag = 0;
    long test;
    if (argc == 1)
    {
        test = 1;
    }
    else
    {
        test = strtol(argv[1], NULL, 10);
    }
    switch (test)
    {
    case 1:
        fd_receiver = llopen(11, RECEIVER);
        printf("did receiver");
        if (fd_receiver == -1)
        {
            printf("Error setting receiver");
        }
        break;
    case 2:
        fd_transmitter = llopen(10, TRANSMITTER);
        printf("did transmitter");

        if (fd_transmitter == -1)
        {
            printf("Error setting transmitter");
        }

        if (llclose(fd_transmitter))
        {
            printf("Error closing receiver");
        }
    default:
        break;
    }
}