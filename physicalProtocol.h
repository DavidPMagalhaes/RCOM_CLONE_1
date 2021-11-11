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
} writeCommandState;

int writeLinkCommand(struct linkLayer *link, char A, char C);

int writeCommandStateMachine(writeCommandState state, char A, char C, char byte);

#endif
