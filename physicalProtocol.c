
#include <signal.h>

#include "physicalProtocol.h"
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

int writeLinkCommand(struct linkLayer *link, char A, char C)
{
    int res;
    struct frame frame = link->frame;
    char byte;
    writeCommandState state = START;

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
        }

        state = writeCommandStateMachine(state, A, C, byte);
        if (state == STOP)
        {
            //Received message successful
            resetalarm();
            return 0;
        }
    }
    return -1; //Couldn't receive an answer
}

int writeCommandStateMachine(writeCommandState state, char A, char C, char byte)
{
    static protectionByte = 0;
    switch (state)
    {
    case START:
        if (byte == F)
        {
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
            return STOP;
        }
        //If I have a bad confirmation should I return an error and retry sending immediately?
        //Or should I wait for the time out before resending?
        return START;
    }
}
