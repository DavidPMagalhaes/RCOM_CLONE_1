#include "physicalProtocol.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "commandMessages.h"

int flag = 1, count = 0;

void atende() // atende alarme
{
    printf("Alarme #%d\n", count);
    flag = 1;
    count++;
}

void resetatende()
{
    printf("Alarm resetted\n");
    (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao
}
void resetalarm()
{
    printf("Reseting alarm\n");
    (void)signal(SIGALRM, resetatende); // instala  rotina que atende interrupcao
}

void writeLinkResponse(struct linkLayer *link)
{
    int res;
    struct frame frame = link->frame;
    res = write(link->fd, frame.stuffedFrame, frame.stuffedFrameSize);
    if (res == -1)
    {
        printf("Fd writing error\n");
        exit(1);
    }
}

int writeLinkCommand(struct linkLayer *link, char A, char C)
{
    printf("writing link command");
    int res;
    struct frame frame = link->frame;
    char byte;
    commandState state = START;

    count = 0;
    (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao
    while (count < link->numTransmissions)
    {
        if (flag)
        {
            res = write(link->fd, frame.stuffedFrame, frame.stuffedFrameSize);
            if (res == -1)
            {
                printf("Fd writing error\n");
                exit(1);
            }
            alarm(link->timeout);
            flag = 0;
        }

        res = read(link->fd, &byte, 1);
        if (res == 0)
        {
            continue;
        }
        if (res == -1)
        {
            printf("Fd reading error \n");
            exit(1);
        }

        state = commandStateMachine(state, A, C, byte);
        if (state == STOP)
        {
            //Received message successful
            resetalarm();
            return 0;
        }
    }
    return -1; //Couldn't receive an answer
}

int readLinkCommand(struct linkLayer *link, char A, char C)
{
    printf("reading link command");
    fflush(stdout);
    int res;
    struct frame frame = link->frame;
    char byte;
    commandState state = START;

    while (1)
    {
        res = read(link->fd, &byte, 1);
        if (res == 0)
        {
            continue;
        }
        if (res == -1)
        {
            printf("Fd reading error \n");
            exit(1);
        }
        state = commandStateMachine(state, A, C, byte);
        if (state == STOP)
        {
            //Received connection request succesfully
            return 0;
        }
    }
}

int commandStateMachine(commandState state, char A, char C, char byte)
{
    static int protectionByte = 0;
    switch (state)
    {
    case START:
        if (byte == F)
        {
            protectionByte = 0;
            return FLAG_RCV;
        }
        return START;
    case FLAG_RCV:
        if (byte == A)
        {
            protectionByte ^= A;
            return A_RCV;
        }
        return START;
    case A_RCV:
        if (byte == C)
        {
            protectionByte ^= C;
            return C_RCV;
        }
        return START;
    case C_RCV:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return BCC_OK;
        }
        return START;
    case BCC_OK:
        if (byte == F)
        {
            protectionByte = 0;
            return STOP;
        }
        //If I have a bad confirmation should I return an error and retry sending immediately?
        //Or should I wait for the time out before resending?
        return START;
    }
}
