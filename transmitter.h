#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "dataProtocol.h"

int openTransmitter(struct linkLayer *link);

int closeTransmitter(struct linkLayer *link);

#endif