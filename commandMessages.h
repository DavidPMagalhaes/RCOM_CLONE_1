#include <stdio.h>
#include <sys/types.h>

//COMMANDS
#define A 0x03
#define F 0x7e
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR 0x05
#define REJ 0x01
#define CMDSZ 5 //[FLAG,ADDRESS,CMD,BCC,FLAG]



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

void commandMessage(char buf[], char C)
{
    snprintf(buf, 6, "%c%c%c%c%c", F, A, C, A ^ C, F); // does this put the end of line character in the buffer too?
}

void SETMessage(char buf[])
{
    commandMessage(buf, SET);
}

void DISCMessage(char buf[])
{
    commandMessage(buf, DISC);
}

void UAMessage(char buf[])
{
    commandMessage(buf, UA);
}

void RRMessage(char buf[], int R)
{
    commandMessage(buf, RR | (R << 7));
}

void REJMessage(char buf[], int R)
{
    commandMessage(buf, REJ | (R << 7));
}