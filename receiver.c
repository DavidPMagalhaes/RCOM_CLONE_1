#include "receiver.h"

#include "commandMessages.h"
#include "physicalProtocol.h"

int openReceiver(struct linkLayer *link)
{
    int res = readLinkCommand(link, A_EM, SET);
    if (res == 0)
    {
        UAMessage(link->frame.stuffedFrame, A_EM);
        link->frame.stuffedFrameSize = CMDSZ;
        writeLinkResponse(link);
    }
}

int closeReceiver(struct linkLayer *link)
{

    int res = 0;
    char mes[6];

    DISCMessage(link->frame.stuffedFrame, A_REC);
    link->frame.stuffedFrameSize = CMDSZ;
    if (writeLinkCommand(link, A_REC, DISC))
    {
        //There was an error receiving disc
        return -1;
    }

    UAMessage(link->frame.stuffedFrame, A_REC);
    link->frame.stuffedFrameSize = CMDSZ;
    writeLinkResponse(link);

    //If res != 0, then the receiver didn't get the memo to close
    //What should the program do now?
    return 0;
}