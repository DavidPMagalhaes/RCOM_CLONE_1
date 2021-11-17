#include "receiver.h"

#include "commandMessages.h"
#include "physicalProtocol.h"
#include "byteStuffing.h"

int openReceiver(struct linkLayer *link)
{
    int res = readLinkCommand(link, A_EM, SET);
    if (res == 0)
    {
        UAMessage(link->frame.frame, A_EM);
        link->frame.frameUsedSize = CMDSZ;
        writeLinkResponse(link);
        return 0;
    }
    return -1;
}

int readReceiver(struct linkLayer *link, char *buffer)
{
    int res;
    while (1)
    {
        link->frame.frameUsedSize = 0;
        res = readLinkInformation(link, buffer, A_EM);
        if (res == -1)
        {
            // Received message to disconnect
            DISCMessage(link->frame.frame, A_EM);
            link->frame.frameUsedSize = CMDSZ;
            writeLinkResponse(link);
            continue;
        }
        if (res == -2)
        {
            // When res == -2 means that we received the same package twice
            // Let's ask for the one we are waiting for, which should be the current sequenceNumber
            RRMessage(link->frame.frame, link->sequenceNumber);
            link->frame.frameUsedSize = CMDSZ; // Reset the size of the buffer
            writeLinkResponse(link);

            // Go back to reading
            continue;
        }
        if (res == -3)
        {
            // Received UA after disconnect
            return 0;
        }

        // Read successfully the correct package
        res = destuff(&(link->frame), buffer);
        if (res == -1)
        {
            // There was corruption in the data
            REJMessage(link->frame.frame, link->sequenceNumber);
            link->frame.frameUsedSize = CMDSZ; // Reset the size of the buffer
            writeLinkResponse(link);

            // Go back to reading
            continue;
        }
        else
        {
            // Ask for next packages
            link->sequenceNumber = (link->sequenceNumber + 1) % 2;
            RRMessage(link->frame.frame, link->sequenceNumber);
            link->frame.frameUsedSize = CMDSZ; // Reset the size of the buffer
            writeLinkResponse(link);
        }
        return res;
    }
}