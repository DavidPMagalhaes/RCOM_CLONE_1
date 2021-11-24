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

int readReceiver(struct linkLayer *link, u_int8_t *buffer)
{
    int res, Nr, A = A_EM;
    int disconnecting = 0;
    while (1)
    {
        link->frame.frameUsedSize = 0;
        res = readLinkInformation(link, buffer, A, &Nr);
        A = A_EM; // In case after a disc message I happen receiving something that isn't a disc (UA failed for example)
        if (res == -1)
        {
            // Received message to disconnect
            DISCMessage(link->frame.frame, A_REC);
            link->frame.frameUsedSize = CMDSZ;
            writeLinkResponse(link);
            A = A_REC;
            // Instead of returning to the loop after having written writeLinkResponse, I could do a writeLinkCommand and return on success
            disconnecting = 1;
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
            if (disconnecting)
            {
                // Received UA after disconnect
                return 0;
            }
            else
            {
                // Received a random UA. Let's just ignore
                continue;
            }
        }

        // Read successfully the correct package
        res = destuff(&(link->frame), buffer);
        if (res == -1)
        {
            // There was corruption in the data
            if (Nr != link->sequenceNumber)
            {
                // Corrupted but it was from a previous package.
                // Ask for next package
                RRMessage(link->frame.frame, link->sequenceNumber);
                link->frame.frameUsedSize = CMDSZ; // Reset the size of the buffer
                writeLinkResponse(link);
            }
            else
            {
                // The current package was corrupted
                REJMessage(link->frame.frame, link->sequenceNumber);
                link->frame.frameUsedSize = CMDSZ; // Reset the size of the buffer
                writeLinkResponse(link);
            }

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