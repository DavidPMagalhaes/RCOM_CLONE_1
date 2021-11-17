#include "transmitter.h"

#include "commandMessages.h"
#include "physicalProtocol.h"
#include "byteStuffing.h"

int openTransmitter(struct linkLayer *link)
{

    SETMessage(link->frame.frame);
    link->frame.frameUsedSize = CMDSZ;
    printFrame(link->frame.frame, CMDSZ);
    //The fact that I have to remember what to send here is annoying me. Perhaps make the commandMessages return a char[2] and use that for the state machine accordingly?
    return writeLinkCommand(link, A_EM, UA);
}

int writeTransmitter(struct linkLayer *link, char *buffer, int length)
{
    int res;
    IMessage(link->frame.frame, link->sequenceNumber);
    link->frame.frameUsedSize = stuff(&(link->frame), buffer, length);
    res = writeLinkInformation(link, A_EM);
    if (res == 0)
    {
        // Let's only change the sequence number if we were successful
        link->sequenceNumber = (link->sequenceNumber + 1) % 2; // N = 2
        return length;
    }
    return -1;
}

int closeTransmitter(struct linkLayer *link)
{
    int res = 0;
    char mes[6];

    DISCMessage(link->frame.frame, A_EM);
    link->frame.frameUsedSize = CMDSZ;
    printFrame(link->frame.frame, link->frame.frameUsedSize);

    if (writeLinkCommand(link, A_EM, DISC))
    {
        //There was an error receiving disc
        return -1;
    }

    UAMessage(link->frame.frame, A_EM);
    link->frame.frameUsedSize = CMDSZ;
    writeLinkResponse(link);

    //If res != 0, then the receiver didn't get the memo to close
    //What should the program do now?
    return 0;
}
