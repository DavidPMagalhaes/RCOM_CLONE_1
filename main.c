#include "dataProtocol.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "physicalProtocol.h"
#include "commandMessages.h"

int main(int argc, char **argv)
{

    linkType receiver;
    linkType transmitter;
    receiver = RECEIVER;
    transmitter = TRANSMITTER;
    int fd_receiver, fd_transmitter;
    char receiver_buffer[255];
    int flag = 0;
    int res;
    long test;
    int FER_head, FER_data;
    struct PHYSICAL_OPTIONS options = CREATE_PHYSICAL_OPTIONS();
    char message_5[] = "hello there!";
    char message_6[255];
    int message_6_len = 0;

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
                printf("-noAlarms enabled\n");
                options.OPTION_NO_ALARMS = 1;
            }
            else if (!strcmp(argv[i], "-FER"))
            {
                if (i + 2 >= argc)
                {
                    printf("Wrong -FER syntax\n");
                    exit(1);
                }
                options.OPTION_FER = 1;
                options.OPTION_FER_HEAD = (int)strtol(argv[i + 1], NULL, 10);
                options.OPTION_FER_DATA = (int)strtol(argv[i + 2], NULL, 10);
                printf("-FER enabled with head odds %d and data odds %d\n", options.OPTION_FER_HEAD, options.OPTION_FER_DATA);
                i += 2;
            }
        }
    }
    PHYSICAL_PROTOCOL_OPTIONS(options);
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

        res = llwrite(fd_transmitter, message_5, strlen(message_5));
        printf("Transmitter: %s: %d bytes\n", message_5, res);
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
    case 6:
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
                printFrame(receiver_buffer, res);
                printf("Receiver:");
            }
        }
        break;
    case 7:
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

        strcpy(message_6, "A mesasge with '\\0' characters\n");
        message_6_len = strlen(message_6);
        message_6[2] = (char)0x00;
        message_6[4] = (char)F;
        message_6[6] = (char)0x7d;
        res = llwrite(fd_transmitter, message_6, message_6_len);
        printf("Transmitter: %s: %d bytes\n", message_6, res);
        printFrame(message_6, message_6_len);
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