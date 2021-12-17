#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include "dataProtocol.h"

int openTransmitter(struct linkLayer *link);

int writeTransmitter(struct linkLayer *link, u_int8_t *buffer, int length);

int closeTransmitter(struct linkLayer *link);

#endif //TRANSMITTER_H