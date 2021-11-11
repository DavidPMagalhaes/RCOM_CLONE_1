#include "commandMessages.h"

int dataIsViable(char mes[], int size)
{
    {
        char sum = 0;
        for (int i = 0; i < size; i++)
        {
            sum ^= mes[i];
        }
        return sum == 0x00;
    }
}

void printCommand(char buf[])
{
    printf("Command:");
    for (int i = 0; i < CMDSZ; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

void printFrame(char *buf, int bufLen)
{
    printf("Frame: ");
    for (int i = 0; i < bufLen; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

void commandMessage(char buf[], char A, char C)
{
    snprintf(buf, 6, "%c%c%c%c%c", F, A, C, A ^ C, F); // does this put the end of line character in the buffer too?
}

void SETMessage(char buf[])
{
    commandMessage(buf, A_EM, SET);
}

void DISCMessage(char buf[], char A)
{
    commandMessage(buf, A, DISC);
}

void UAMessage(char buf[], char A)
{
    commandMessage(buf, A, UA);
}

void RRMessage(char buf[], int R)
{
    commandMessage(buf, A_EM, RR | (R << 7));
}

void REJMessage(char buf[], int R)
{
    commandMessage(buf, A_EM, REJ | (R << 7));
}