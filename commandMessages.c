#include "commandMessages.h"

void printCommand(u_int8_t buf[])
{
    printf("Command:");
    for (int i = 0; i < CMDSZ; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

void printFrame(u_int8_t *buf, int bufLen)
{
    printf("Frame: ");
    for (int i = 0; i < bufLen; i++)
    {
        printf("%02x", buf[i]);
    }
    printf("\n");
}

void commandMessage(u_int8_t buf[], u_int8_t A, u_int8_t C)
{
    snprintf(buf, 6, "%c%c%c%c%c", F, A, C, A ^ C, F); // does this put the end of line character in the buffer too?
}

void SETMessage(u_int8_t buf[])
{
    commandMessage(buf, A_EM, SET);
}

void DISCMessage(u_int8_t buf[], u_int8_t A)
{
    commandMessage(buf, A, DISC);
}

void UAMessage(u_int8_t buf[], u_int8_t A)
{
    commandMessage(buf, A, UA);
}

void RRMessage(u_int8_t buf[], int R)
{
    commandMessage(buf, A_EM, RR | (R << 7));
}

void REJMessage(u_int8_t buf[], int R)
{
    commandMessage(buf, A_EM, REJ | (R << 7));
}

void IMessage(u_int8_t buf[], int S){
    u_int8_t C = S << 6; // Does this work?
    snprintf(buf, 5, "%c%c%c%c", F, A_EM, C, A_EM ^ C);
}