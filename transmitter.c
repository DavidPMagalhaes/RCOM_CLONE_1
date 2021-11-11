#include "transmitter.h"

#include "commandMessages.h"
#include "physicalProtocol.h"

int openTransmitter(struct linkLayer *link)
{

    SETMessage(link->frame.stuffedFrame);
    link->frame.stuffedFrameSize = CMDSZ;
    printFrame(link->frame.stuffedFrame, link->frame.stuffedFrameSize);
    //The fact that I have to remember what to send here is annoying me. Perhaps make the commandMessages return a char[2] and use that for the state machine accordingly?
    return writeLinkCommand(link, A_EM, UA);
}

int closeTransmitter(struct linkLayer *link)
{
    int res = 0;
    char mes[6];

    DISCMessage(link->frame.stuffedFrame, A_EM);
    link->frame.stuffedFrameSize = CMDSZ;
    printFrame(link->frame.stuffedFrame, link->frame.stuffedFrameSize);

    if (writeLinkCommand(link, A_EM, DISC))
    {
        //There was an error receiving disc
        return -1;
    }

    UAMessage(link->frame.stuffedFrame, A_EM);
    link->frame.stuffedFrameSize = CMDSZ;
    writeLinkResponse(link);

    //If res != 0, then the receiver didn't get the memo to close
    //What should the program do now?
    return 0;
}
