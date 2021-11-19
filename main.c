#include "dataProtocol.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>


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
    char message_7[255];
    int message_7_len = 0;
    u_int8_t message_8[255];
    int message_8_len = 0;
    int message_8_rep = 30;
    time_t random_seed_8;

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
                printf("Receiver:");
                printFrame(receiver_buffer, res);
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

        strcpy(message_7, "A message with '\\0' characters\n");
        message_7_len = strlen(message_7);
        message_7[2] = 0x00;
        message_7[4] = F;
        message_7[6] = 0x7d;
        res = llwrite(fd_transmitter, message_7, message_7_len);
        printf("Transmitter: %s: %d bytes\n", message_7, res);
        printFrame(message_7, message_7_len);
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
    case 8:
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
        random_seed_8 = time(NULL);
        random_seed_8 = 1637263042;
        printf("%ld", random_seed_8);
        srand(random_seed_8);
        message_8_len = 255;
        for (int i = 0; i < message_8_rep; i++)
        {
            for (int j = 0; j < message_8_len; j++)
            {
                message_8[j] = (rand() % 256);
            }
            res = llwrite(fd_transmitter, message_8, message_8_len);
            printf("Transmitter: %d bytes\n", res);
            printFrame(message_8, message_8_len);
        }
        printf("Transmitter: Asking to close\n");
        if (llclose(fd_transmitter))
        {
            printf("Transmitter: Error closing transmitter\n");
        }
        else
        {
            printf("Transmitter: Successfully closed transmitter\n");
        }
        printf("%ld", random_seed_8);
        break;
    default:
        break;
    }
}