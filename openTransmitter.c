
#include "openTransmitter.h"
#include "commandMessages.h"

int openTransmiter(struct linkLayer *link)
{
    char mes[255], buf[255];

    SETMessage(link->frame.stuffedFrame);
    link->frame.stuffedFrameSize = CMDSZ;

    //The fact that I have to remember what to send here is annoying me. Perhaps make the commandMessages return a char[2] and use that for the state machine accordingly?
    return writeLinkCommand(link, A_EM, SET); 
    
}
