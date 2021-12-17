#include "physicalProtocol.h"

#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

#include "commandMessages.h"
#include "options.h"

int flag = 1;
int count = 0;
int flagDisc = 0;

void atende() // atende alarme
{
    if (OPTIONS_ALARM())
    {
        printf("Alarme #%d\n", count);
        flag = 1;
        count++;
    }
}

void atendeDisc()
{
    if (OPTIONS_ALARM())
    {
        flagDisc = 1;
    }
}

int FdWrite(int fd, void *buf, size_t size)
{
    if (OPTIONS_PACKET_LOSS())
    {
        return size; // Returns count because, as far as the transmitter knows, it was successful
    }
    return write(fd, buf, size);
}

void writeLinkResponse(struct linkLayer *link)
{
    int res;
    struct frame frame = link->frame;
    res = FdWrite(link->fd, link->frame.frame, link->frame.frameUsedSize);
    if (res == -1)
    {
        printf("Fd writing error\n");
        exit(1);
    }
}

int writeLinkCommand(struct linkLayer *link, u_int8_t A, u_int8_t C)
{
    int res;
    struct frame frame = link->frame;
    u_int8_t byte;
    commandState state = START;
    flag = 1;
    count = 0;
    (void)signal(SIGALRM, atende);
    while (count < link->numTransmissions)
    {
        if (flag)
        {
            flag = 0;
            alarm(link->timeout);
            OPTIONS_TPROP();
            res = FdWrite(link->fd, frame.frame, link->frame.frameUsedSize);

            if (res == -1)
            {
                printf("Fd writing error\n");
                exit(1);
            }
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

int writeLinkInformation(struct linkLayer *link, u_int8_t A)
{
    int res, Nr = 0;
    struct frame frame = link->frame;
    u_int8_t byte;
    flag = 1;
    writeInformationState state = WI_START;

    count = 0;
    (void)signal(SIGALRM, atende);
    while (count < link->numTransmissions)
    {
        if (flag)
        {
            flag = 0;
            alarm(link->timeout);
            OPTIONS_TPROP();
            res = FdWrite(link->fd, frame.frame, link->frame.frameUsedSize);

            if (res == -1)
            {
                printf("Fd writing error\n");
                exit(1);
            }
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
        if (state == WI_STOP_REJ) // Rejection message
        {
            if (Nr == link->sequenceNumber) //Information referring to this frame
            {
                // We will need to retransmit
                alarm(0);         // Cancel scheduled alarm
                count = 0;        // Reset number of attempts because receiver is active
                flag = 1;         // Flag to write again
                state = WI_START; // Set the state to the start
                continue;
            }
            else
            {
                // Probably a delayed answer, but we will retransmit.
                alarm(0);         // Cancel scheduled alarm
                count = 0;        // Reset number of attempts because receiver is active
                flag = 1;         // Flag to write again
                state = WI_START; // Set the state to the start
                continue;
            }
        }
        else if (state == WI_STOP_RR) //Receiver ready message
        {
            if (Nr != link->sequenceNumber) //Is asking for the next frame. All ok
            {
                //Received message successful
                alarm(0);
                return 0;
            }
            else //Duplicate
            {
                // Will retransmit current package
                alarm(0);         // Cancel scheduled alarm
                count = 0;        // Reset number of attempts because receiver is active
                flag = 1;         // Flag to write again
                state = WI_START; // Set the state to the start
                continue;
            }
        }
    }
    return -1; //Couldn't receive an answer Timeout
}

int readLinkCommand(struct linkLayer *link, u_int8_t A, u_int8_t C)
{
    int res;
    struct frame frame = link->frame;
    u_int8_t byte;
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
            //Received command confirmation successfully
            return 0;
        }
    }
}

int readLinkInformation(struct linkLayer *link, u_int8_t A, int *Nr)
{
    int res;
    struct frame frame = link->frame;
    u_int8_t byte;
    readInformationState state = RI_START;

    if (A == A_REC)
    {
        // Received message to disconnect. If there is no confirmation will timeout as if it had
        flagDisc = 0;
        (void)signal(SIGALRM, atendeDisc);
        alarm(link->timeout);
    }
    while (1)
    {
        res = read(link->fd, &byte, 1);

        if (flagDisc)
        {
            flagDisc = 0;
            alarm(0);
            return -3;
        }
        if (res == 0)
        {
            continue;
        }
        if (res == -1)
        {
            printf("Fd reading error \n");
            exit(1);
        }
        if (link->frame.frameUsedSize == MAX_BUFFER_SIZE)
        {
            return -2;
        }
        link->frame.frame[link->frame.frameUsedSize] = byte;
        link->frame.frameUsedSize++;
        state = readInformationStateMachine(state, A, byte, Nr);

        // Generating errors.
        OPTIONS_GENERATE_FER(link, &state, A, Nr);

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

int commandStateMachine(commandState state, u_int8_t A, u_int8_t C, u_int8_t byte)
{
    static u_int8_t protectionByte = 0;
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
            protectionByte = 0;
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
            protectionByte = 0;
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
            protectionByte = 0;
            return FLAG_RCV;
        }
        return START;
    case BCC_OK:
        if (byte == F)
        {
            protectionByte = 0;
            return STOP;
        }
        return START;
    }
}

int writeInformationStateMachine(writeInformationState state, u_int8_t A, u_int8_t byte, int *Nr)
{
    static u_int8_t protectionByte = 0;
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
            protectionByte = 0;
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_A_RCV:
        (*Nr) = byte >> 7;

        protectionByte ^= byte;
        if (byte == REJ || byte == REJ_N1)
        {
            return WI_REJ_RCV;
        }
        else if (byte == RR || byte == RR_N1)
        {
            return WI_RR_RCV;
        }
        if (byte == F)
        {
            protectionByte = 0;
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
            protectionByte = 0;
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
            protectionByte = 0;
            return WI_FLAG_RCV;
        }
        return WI_START;
    case WI_BCC_REJ_OK:
        if (byte == F)
        {
            return WI_STOP_REJ;
        }
        return WI_START;
    case WI_BCC_RR_OK:
        if (byte == F)
        {
            return WI_STOP_RR;
        }
        return WI_START;
    }
}

int readInformationStateMachine(readInformationState state, u_int8_t A, u_int8_t byte, int *Nr)
{
    static u_int8_t protectionByte = 0;

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
        if (byte == A_REC || byte == A_EM)
        {
            protectionByte ^= byte;
            return RI_A_RCV;
        }
        if (byte == F)
        {
            protectionByte = 0;
            return RI_FLAG_RCV;
        }
        return RI_RESET;
    case RI_A_RCV:
        (*Nr) = byte >> 6;
        protectionByte ^= byte;
        if (byte == 0 || byte == (1 << 6))
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
            protectionByte = 0;
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
            protectionByte = 0;
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
            protectionByte = 0;
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
            protectionByte = 0;
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

    default:
        return RI_START;
    }
}