#include "byteStuffing.h"

#include <stdlib.h>
#include <string.h>
#include "commandMessages.h"

// TODO Teacher
// Stuffing and protection byte should be assembled seperately

int stuff(struct frame *frame, u_int8_t tostuff[], int tostuffSize)
{
    // Tostuff will not overflow the buffer

    int sfCount = FIRST_DATA_INDEX; //stuffed count
    int nsfCount = 0;               //nonstuffed count
    int flagged = 0;
    u_int8_t c, protectionByte = 0;

    for (int i = 0; i < tostuffSize; i++)
    {
        c = tostuff[i];
        protectionByte ^= c;
        if (c == ESCAPE)
        {
            frame->frame[sfCount] = ESCAPE;
            frame->frame[sfCount + 1] = ESCAPE_THE_ESCAPE;
            sfCount += 2;
        }
        else if (c == F)
        {
            frame->frame[sfCount] = ESCAPE;
            frame->frame[sfCount + 1] = ESCAPE_THE_FLAG;
            sfCount += 2;
        }
        else
        {
            frame->frame[sfCount] = c;
            sfCount++;
        }
    }

    if (protectionByte == F) //Special and rare case
    {

        frame->frame[sfCount] = ESCAPE;
        sfCount++;
        frame->frame[sfCount] = ESCAPE_THE_FLAG;
        sfCount++;
        frame->frame[sfCount] = F;
        sfCount++;
    }
    else if (protectionByte == ESCAPE)
    {
        frame->frame[sfCount] = ESCAPE;
        sfCount++;
        frame->frame[sfCount] = ESCAPE_THE_ESCAPE;
        sfCount++;
        frame->frame[sfCount] = F;
        sfCount++;
    }
    else
    {
        frame->frame[sfCount] = protectionByte;
        sfCount++;
        frame->frame[sfCount] = F;
        sfCount++;
    }
    return sfCount;
}

int destuff(struct frame *frame, u_int8_t *buffer)
{
    //Previous functions should have checked the first protection byte
    //We will only check the second

    // if (bufferCapacity == 0)
    // {
    //     allocBufferSpace(buffer, bufferCapacity, SPACE_ALLOC_SIZE);
    // }

    int sfcount = FIRST_DATA_INDEX;
    int nsfCount = 0;
    u_int8_t c, protectionByte = 0;

    for (int i = FIRST_DATA_INDEX; frame->frame[i] != F; i++)
    {
        // if (nsfCount == bufferCapacity)
        // {
        //     allocBufferSpace(buffer, bufferCapacity, SPACE_ALLOC_SIZE);
        // }
        c = frame->frame[i];
        if (c == ESCAPE)
        {
            i++;
            c = frame->frame[i];
            if (c == ESCAPE_THE_FLAG)
            {
                buffer[nsfCount] = F;
                protectionByte ^= F;
            }
            else if (c == ESCAPE_THE_ESCAPE)
            {
                buffer[nsfCount] = ESCAPE;
                protectionByte ^= ESCAPE;
            }
            else
            { //After an escape character there must always be one of those two
                return -1;
            }
        }
        else
        {
            buffer[nsfCount] = c;
            protectionByte ^= c;
        }
        nsfCount++;
    }

    if (protectionByte != 0) //Data was corrupted
    {
        return -1; //Error: Please resend
    }
    else // All good
    {
        return nsfCount - 1; //-1 because the protection byte that was also in the stuffed sequence doesn't belong to the data
    }
}