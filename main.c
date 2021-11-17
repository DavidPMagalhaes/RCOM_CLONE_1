#include "dataProtocol.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "physicalProtocol.h"
int main(int argc, char **argv)
{

    linkType receiver;
    linkType transmitter;
    receiver = RECEIVER;
    transmitter = TRANSMITTER;
    int fd_receiver, fd_transmitter;
    char receiver_buffer[255];
    int flag = 0, bitmask = 0;
    int res;
    long test;
    char message[] = "hello there!";
    if (argc == 1)
    {
        test = 1;
    }
    else
    {
        test = strtol(argv[1], NULL, 10);
        for (int i = 2; i < argc; i++)
        {
            printf("%s\n", argv[i]);
            if (!strcmp(argv[i], "-noAlarms"))
            {
                bitmask |= (1 << 0);
            }
        }
    }
    programOptions(bitmask);
    switch (test)
    {
    case 1:
        printf("Receiver: opening receiver\n");
        fd_receiver = llopen(11, RECEIVER);
        if (fd_receiver == -1)
        {
            printf("Error setting receiver\n");
        }
        else
        {
            printf("Receiver: Receiver opened\n");
        }
        printf("Receiver: Reading...\n");
        res = llread(fd_receiver, receiver_buffer);
        if (res == 0)
        {
            printf("Receiver: Disconnected successfully\n");
        }

        break;
    case 2:
        fd_transmitter = llopen(10, TRANSMITTER);
        if (fd_transmitter == -1)
        {
            printf("Transmitter: Error setting transmitter\n");
        }
        else
        {
            printf("Transmitter: Transmitter opened\n");
        }
        usleep(100 * 1000);
        printf("Transmitter: Asking to close\n");
        if (llclose(fd_transmitter))
        {
            printf("Transmitter: Error closing transmitter\n");
        }
        else
        {
            printf("Transmitter: Successfully closed transmitter\n");
        }
        break;
    case 3:
        fd_transmitter = llopen(10, TRANSMITTER);
        if (fd_transmitter == -1)
        {
            printf("Transmitter: Error setting transmitter\n");
        }
        else
        {
            printf("Transmitter: Transmitter opened\n");
        }
        break;
    case 4:
        printf("Receiver: opening receiver\n");
        fd_receiver = llopen(11, RECEIVER);
        if (fd_receiver == -1)
        {
            printf("Error setting receiver\n");
        }
        else
        {
            printf("Receiver: Receiver opened\n");
        }
        printf("Receiver: Reading...\n");
        while (1)
        {
            res = llread(fd_receiver, receiver_buffer);
            if (res == 0)
            {
                printf("Receiver: Disconnected successfully\n");
                break;
            }
            else
            {
                printf("Receiver: %s\n", receiver_buffer);
            }
        }
        break;
    case 5:
        fd_transmitter = llopen(10, TRANSMITTER);
        if (fd_transmitter == -1)
        {
            printf("Transmitter: Error setting transmitter\n");
        }
        else
        {
            printf("Transmitter: Transmitter opened\n");
        }
        usleep(100 * 1000);

        res = llwrite(fd_transmitter, message, strlen(message));
        printf("Transmitter: %s: %d bytes\n", message, res);
        usleep(100 * 1000);
        printf("Transmitter: Asking to close\n");
        if (llclose(fd_transmitter))
        {
            printf("Transmitter: Error closing transmitter\n");
        }
        else
        {
            printf("Transmitter: Successfully closed transmitter\n");
        }
        break;
    default:
        break;
    }
}