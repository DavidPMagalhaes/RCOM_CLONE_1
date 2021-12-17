#include "receiver.h"

#include <string.h>
#include "commandMessages.h"
#include "physicalProtocol.h"
#include "byteStuffing.h"
#include "dataProtection.h"

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
    int corrupted = 0, verifyBcc = 0;
    u_int8_t bufferWithBcc[MAX_SIZE + 2];
    int disconnecting = 0;
    while (1)
    {
        corrupted = 0;
        link->frame.frameUsedSize = 0;
        res = readLinkInformation(link, A, &Nr);
        if (res == -1)
        {
            // Received message to disconnect
            DISCMessage(link->frame.frame, A_REC);
            link->frame.frameUsedSize = CMDSZ;
            writeLinkResponse(link);
            A = A_REC;
            disconnecting = 1;
            continue;
        }
        if (res == -2)
        {
            // Received duplicate package. Ask for current package
            RRMessage(link->frame.frame, link->sequenceNumber);
            link->frame.frameUsedSize = CMDSZ;
            writeLinkResponse(link);
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
        // Destuff to a buffer with one extra space for the bcc character
        res = destuff(&(link->frame), bufferWithBcc);
        if (res == -1)
        {
            // Wrong character found after escape character
            corrupted = 1;
        }
        else //Destuffing was ok
        {
            // Verify protection byte
            verifyBcc = verifyProtectionByte(bufferWithBcc, res);
        }

        if (!verifyBcc || corrupted)
        {
            // Was already corrupted or the bcc was wrong
            corrupted = 1;
        }
        else // Protection byte was ok
        {
            // Copy to the buffer return buffer
            res -= 1; // Res included the protection byte. The message is in fact one byte shorter
            memcpy(buffer, bufferWithBcc, res);
        }
        if (corrupted)
        {
            // There was corruption in the data
            if (Nr != link->sequenceNumber)
            {
                // Corrupted but it was from a previous package.
                // Ask for current package
                RRMessage(link->frame.frame, link->sequenceNumber);
                link->frame.frameUsedSize = CMDSZ;
                writeLinkResponse(link);
            }
            else
            {
                // The current package was corrupted
                REJMessage(link->frame.frame, link->sequenceNumber);
                link->frame.frameUsedSize = CMDSZ; 
                writeLinkResponse(link);
            }

            // Go back to reading
            continue;
        }
        else
        {
            // Ask for next package
            link->sequenceNumber = (link->sequenceNumber + 1) % 2;
            RRMessage(link->frame.frame, link->sequenceNumber);
            link->frame.frameUsedSize = CMDSZ; 
            writeLinkResponse(link);
        }
        return res;
    }
}