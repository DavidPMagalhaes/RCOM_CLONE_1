#ifndef PHYSICALPROTOCOL_H
#define PHYSICALPROTOCOL_H

#include "dataProtocol.h"

typedef enum
{
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} commandState;

void writeLinkResponse(struct linkLayer *link);

int writeLinkCommand(struct linkLayer *link, char A, char C);

int readLinkCommand(struct linkLayer *link, char A, char C);

int commandStateMachine(commandState state, char A, char C, char byte);

#endif
