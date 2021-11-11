#include "byteStuffing.h"

#include <stdlib.h>
#include <string.h>
#include "commandMessages.h"

int stuff(struct frame *frame, char tostuff[], int tostuffSize)
{
    if (frame->stuffedFrameSize == 0)
    {
        initializeFrame(frame); //Makes size of the stuffed frame to be MAX_SIZE
    }
    int sfCount = FIRST_DATA_INDEX; //stuffed count
    int nsfCount = 0;               //nonstuffed count
    int flagged = 0;
    char c, protectionByte = 0;

    for (int i = 0; i < tostuffSize; i++)
    {

        //Gets 20 more bytes of space in the stuffedFrame
        if (sfCount == frame->stuffedFrameSize - 2) //-2 because that's the maximum amount of space that we will add in an iteration
        {
            allocSpace(frame, SPACE_ALLOC_SIZE);
        }

        c = tostuff[i];
        protectionByte ^= c;
        if (c == ESCAPE)
        {
            frame->stuffedFrame[sfCount] = ESCAPE;
            frame->stuffedFrame[sfCount + 1] = ESCAPE_THE_ESCAPE;
            sfCount += 2;
        }
        else if (c == F)
        {
            frame->stuffedFrame[sfCount] = ESCAPE;
            frame->stuffedFrame[sfCount + 1] = ESCAPE_THE_FLAG;
            sfCount += 2;
        }
        else
        {
            frame->stuffedFrame[sfCount] = c;
            sfCount++;
        }
    }

    if (protectionByte == F) //Special and rare case
    {
        if (sfCount + 3 > frame->stuffedFrameSize) // the previous operation filled the buffer
        {
            allocSpace(frame, 3);
        }
        frame->stuffedFrame[sfCount] = ESCAPE;
        sfCount++;
        frame->stuffedFrame[sfCount] = ESCAPE_THE_FLAG;
        sfCount++;
        frame->stuffedFrame[sfCount] = F;
        sfCount++;
    }
    else
    {
        if (sfCount + 2 > frame->stuffedFrameSize) // the previous operation filled the buffer
        {
            allocSpace(frame, 2);
        }
        frame->stuffedFrame[sfCount] = protectionByte;
        sfCount++;
        frame->stuffedFrame[sfCount] = F;
        sfCount++;
    }
    return 0;
}

int destuff(struct frame *frame, char *buffer, int bufferCapacity, int *bufferLength)
{
    //Previous functions should have checked the first protection byte
    //We will only check the second

    if (frame->stuffedFrameSize == 0)
    {
        initializeFrame(frame);
    }

    if (bufferCapacity == 0)
    {
        allocBufferSpace(buffer, bufferCapacity, SPACE_ALLOC_SIZE);
    }

    int sfcount = FIRST_DATA_INDEX;
    int nsfCount = 0;
    char c, protectionByte = 0;

    for (int i = FIRST_DATA_INDEX; frame->stuffedFrame[i] != F; i++)
    {
        if (nsfCount == bufferCapacity)
        {
            allocBufferSpace(buffer, bufferCapacity, SPACE_ALLOC_SIZE);
        }
        c = frame->stuffedFrame[i];
        if (c == ESCAPE)
        {
            i++;
            c = frame->stuffedFrame[i];
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

void allocSpace(struct frame *frame, int space)
{
    char *newFrame = (char *)malloc(frame->stuffedFrameSize + space);
    memcpy((void *)newFrame, (void *)frame->stuffedFrame, frame->stuffedFrameSize);
    free(frame->stuffedFrame);
    frame->stuffedFrame = newFrame;
    frame->stuffedFrameSize += space;
}

int allocBufferSpace(char *buffer, int prevSpace, int extraSpace)
{
    int totalSpace = prevSpace + extraSpace;
    char *newBuf = (char *)malloc(totalSpace);
    memcpy(newBuf, buffer, prevSpace);
    free(buffer);
    buffer = newBuf;
    return totalSpace;
}