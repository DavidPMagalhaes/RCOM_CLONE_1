#include "commandMessages.h"

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

void IMessage(char buf[], int S){
    u_int8_t C = S << 6; // Does this work?
    snprintf(buf, 5, "%c%c%c%c", F, A_EM, C, A_EM ^ C);
}