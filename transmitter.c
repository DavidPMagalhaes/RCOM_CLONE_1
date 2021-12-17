#include "transmitter.h"

#include "commandMessages.h"
#include "physicalProtocol.h"
#include "byteStuffing.h"
#include "dataProtection.h"

int openTransmitter(struct linkLayer *link)
{

    SETMessage(link->frame.frame);
    link->frame.frameUsedSize = CMDSZ;
    return writeLinkCommand(link, A_EM, UA);
}

int writeTransmitter(struct linkLayer *link, u_int8_t *buffer, int length)
{
    int res;
    u_int8_t protectionByte;
    IMessage(link->frame.frame, link->sequenceNumber);
    protectionByte = createProtectionByte(buffer,length);
    link->frame.frameUsedSize = stuff(&(link->frame), buffer, length, protectionByte);
    res = writeLinkInformation(link, A_EM);
    if (res == 0)
    {
        link->sequenceNumber = (link->sequenceNumber + 1) % 2; // Changes the sequence number. N = 2
        return length; // Return number of bytes transmitted
    }
    return -1;
}

int closeTransmitter(struct linkLayer *link)
{
    int res = 0;
    u_int8_t mes[6];

    DISCMessage(link->frame.frame, A_EM);
    link->frame.frameUsedSize = CMDSZ;

    if (writeLinkCommand(link, A_REC, DISC))
    {
        //Timeout receiving disc
        return -1;
    }

    UAMessage(link->frame.frame, A_REC);
    link->frame.frameUsedSize = CMDSZ;
    writeLinkResponse(link);

    return 0;
}
