#include "physicalProtocol.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include "time.h" //For FER
#include "commandMessages.h"

int flag = 1, count = 0;
struct PHYSICAL_OPTIONS OPTIONS;

struct PHYSICAL_OPTIONS CREATE_PHYSICAL_OPTIONS()
{
    struct PHYSICAL_OPTIONS options = {0, 0, 0, 0};
    srand(time(NULL));
    return options;
}

void
OPTIONS_GENERATE_FER(struct linkLayer *link)
{
    int r;
    // Errors for head
    if (OPTIONS.OPTION_FER_HEAD != 0)
    {
        for (int i = 1; i <= 3; i++)
        {
            // Head is beteeen indices 1 and 3 inclusively
            r = rand() % OPTIONS.OPTION_FER_HEAD;
            if (!r)
            {
                // 1 in a OPTIONS_FER_DATA chance of being a 0
                // We will totally switch the byte.
                link->frame.frame[i] = link->frame.frame[i] ^ 0xff;
            }
        }
    }

    // Errors for data
    if (OPTIONS.OPTION_FER_DATA != 0)
    {
        for (int i = 4; i < link->frame.frameUsedSize - 1; i++)
        {
            // Creating errors in the information and bcc2 bytes. Flag is ignored hence -1
            // Keep in mind we are creating errors in the stuffed message. Simulating errors in transmissions
            r = rand() % OPTIONS.OPTION_FER_DATA;
            if (!r)
            {
                // 1 in a OPTIONS_FER_DATA chance of being a 0
                // We will totally switch the byte.
                link->frame.frame[i] = link->frame.frame[i] ^ 0xff;
            }
        }
    }
}

void atende() // atende alarme
{
    if (OPTIONS.OPTION_NO_ALARMS)
    {
        return;
    }
    printf("Alarme #%d\n", count);
    flag = 1;
    count++;
}

void PHYSICAL_PROTOCOL_OPTIONS(struct PHYSICAL_OPTIONS cmd_options)
{
    OPTIONS = cmd_options;
}

void writeLinkResponse(struct linkLayer *link)
{
    int res;
    struct frame frame = link->frame;
    res = write(link->fd, frame.frame, frame.frameUsedSize);
    if (res == -1)
    {
        printf("Fd writing error\n");
        exit(1);
    }
}

int writeLinkCommand(struct linkLayer *link, char A, char C)
{
    // printf("writing link command");
    int res;
    struct frame frame = link->frame;
    char byte;
    commandState state = START;
    flag = 1;
    count = 0;
    (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao
    while (count < link->numTransmissions)
    {
        if (flag)
        {
            res = write(link->fd, frame.frame, link->frame.frameUsedSize);
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
            alarm(0);
            return 0;
        }
    }
    return -1; //Couldn't receive an answer Timeout
}

int writeLinkInformation(struct linkLayer *link, char A)
{
    // printf("writing link command");
    int res, Nr = 0;
    struct frame frame = link->frame;
    char byte;
    flag = 1;
    writeInformationState state = WI_START;

    count = 0;
    (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao
    while (count < link->numTransmissions)
    {
        if (flag)
        {
            res = write(link->fd, frame.frame, link->frame.frameUsedSize);
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

        state = writeInformationStateMachine(state, A, byte, &Nr);
        if (state == WI_STOP_REJ)
        {
            if (Nr == link->sequenceNumber) //Information referring to this frame
            {
                // We will need to retransmit
                // We will reset the timeout and the attempts as if we had just received the llwrite from the application layer
                alarm(0);         // Cancel scheduled alarm
                count = 0;        // Reset number of attempts
                flag = 1;         // Flag to write again
                state = WI_START; // Set the state to the start
                continue;

                //////////////// 2ND OPTION
                // Therefore, we do nothing so the alarm signal will just make us write again
                // We are considering that rejecting is the same as a time out in terms of just retransmitting but increasing to the count
                // Otherwise we could endlessly receive rej messages
                // state = WI_START;
            }
            else
            {
                state = WI_START;
                // Probably a delayed answer. We will just ignore
            }
        }
        else if (state == WI_STOP_RR)
        {
            if (Nr != link->sequenceNumber) //Is asking for the next frame. All ok
            {
                //Received message successful
                alarm(0);
                return 0;
            }
            else
            {
                state = WI_START;
                // Ignore
            }
        }
    }
    return -1; //Couldn't receive an answer Timeout
}

int readLinkCommand(struct linkLayer *link, char A, char C)
{
    // printf("reading link command");
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

int readLinkInformation(struct linkLayer *link, char *buffer, char A, int *Nr)
{
    // printf("reading link command");
    fflush(stdout);
    int res;
    struct frame frame = link->frame;
    char byte;
    readInformationState state = RI_START;
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
        link->frame.frame[link->frame.frameUsedSize] = byte;
        link->frame.frameUsedSize++;
        state = readInformationStateMachine(state, A, byte, Nr);

        // Enforcing FER Generation will reprocess the head after errors might have been added to it
        if (state == RI_INFORMATION_STOP)
        {
            if (OPTIONS.OPTION_FER)
            {
                OPTIONS_GENERATE_FER(link);
                state = RI_START;
                for (int i = 0; i < link->frame.frameUsedSize; i++)
                {
                    byte = link->frame.frame[i];
                    state = readInformationStateMachine(state, A, byte, Nr);
                }
            }
        }
        if (state == RI_INFORMATION_STOP)
        {

            if ((*Nr) == link->sequenceNumber)
            {
                // Correct one
                // Received information successfully
                return 0;
            }
            else
            {
                // Duplicate package
                return -2;
            }
        }
        else if (state == RI_READ_STOP)
        {
            // Received a message to disconnect
            return -1;
        }
        else if (state == RI_READ_STOP_UA)
        {
            // Confirm disconenct
            return -3;
        }
        else if (state == RI_RESET)
        {
            link->frame.frameUsedSize = 0; // Reset the size of the buffer
            state = RI_START;
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
        protectionByte ^= byte;
        if (byte == A)
        {
            return A_RCV;
        }
        if (byte == F)
        {
            return FLAG_RCV;
        }
        return START;
    case A_RCV:
        protectionByte ^= C;
        if (byte == C)
        {
            return C_RCV;
        }
        if (byte == F)
        {
            return FLAG_RCV;
        }
        return START;
    case C_RCV:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return BCC_OK;
        }
        if (byte == F)
        {
            return FLAG_RCV;
        }
        // If not should I jump to a BCC_NOT_OK instead?
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

int writeInformationStateMachine(writeInformationState state, char A, char byte, int *Nr)
{
    u_int8_t b;
    static int protectionByte = 0;
    switch (state)
    {
    case WI_START:
        if (byte == F)
        {
            protectionByte = 0;
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_FLAG_RCV:
        protectionByte ^= byte;
        if (byte == A)
        {
            return WI_A_RCV;
        }
        if (byte == F)
        {
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_A_RCV:
        b = (u_int8_t)byte;
        (*Nr) = b >> 7;

        protectionByte ^= byte;
        if (b == REJ || b == REJ_N1)
        {
            return WI_REJ_RCV;
        }
        else if (b == RR || b == RR_N1)
        {
            return WI_RR_RCV;
        }
        if (byte == F)
        {
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_REJ_RCV:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return WI_BCC_REJ_OK;
        }
        if (byte == F)
        {
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_RR_RCV:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return WI_BCC_RR_OK;
        }
        if (byte == F)
        {
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_BCC_REJ_OK:
        if (byte == F)
        {
            return WI_STOP_REJ;
        }
        //If I have a bad confirmation should I return an error and retry sending immediately?
        //Or should I wait for the time out before resending?
        return WI_START;
    case WI_BCC_RR_OK:
        if (byte == F)
        {
            return WI_STOP_RR;
        }
        return WI_START;
    }
}

int readInformationStateMachine(readInformationState state, char A, char byte, int *Nr)
{

    u_int8_t b;
    static int protectionByte = 0;

    switch (state)
    {
    case RI_START:
        if (byte == F)
        {
            protectionByte = 0;
            return RI_FLAG_RCV;
        }
        return RI_START;
    case RI_FLAG_RCV:
        if (byte == A)
        {
            protectionByte ^= byte;
            return RI_A_RCV;
        }
        if (byte == F)
        {
            return RI_FLAG_RCV;
        }
        return RI_RESET;
    case RI_A_RCV:
        b = (u_int8_t)byte;
        (*Nr) = b >> 6;
        protectionByte ^= byte;
        // Might be a error here because byte is not a u_int8_t and stuff happens
        if (b == 0 || b == (1 << 6))
        {
            return RI_INF;
        }
        if (byte == DISC)
        {
            return RI_DISC;
        }
        if (byte == UA)
        {
            return RI_UA;
        }
        if (byte == F)
        {
            return RI_FLAG_RCV;
        }
        return RI_RESET;
    case RI_INF:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return RI_INFORMATION_READ;
        }
        if (byte == F)
        {
            return RI_FLAG_RCV;
        }
        return RI_RESET;
    case RI_DISC:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return RI_BCC_DISC_OK;
        }
        if (byte == F)
        {
            return RI_FLAG_RCV;
        }
        return RI_RESET;
    case RI_UA:
        protectionByte ^= byte;
        if (protectionByte == 0)
        {
            return RI_BCC_UA_OK;
        }
        if (byte == F)
        {
            return RI_FLAG_RCV;
        }
        return RI_RESET;
    case RI_BCC_DISC_OK:
        if (byte == F)
        {
            return RI_READ_STOP;
        }
        return RI_RESET;
    case RI_BCC_UA_OK:
        if (byte == F)
        {
            return RI_READ_STOP_UA;
        }
        return RI_RESET;
    case RI_INFORMATION_READ:
        if (byte == F)
        {
            return RI_INFORMATION_STOP;
        }
        return RI_INFORMATION_READ;
    }
}