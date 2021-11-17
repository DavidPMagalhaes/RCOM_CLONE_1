#ifndef RECEIVER_H
#define RECEIVER_H

#include "dataProtocol.h"

int openReceiver(struct linkLayer *link);

int readReceiver(struct linkLayer *link, char *buffer);

#endif