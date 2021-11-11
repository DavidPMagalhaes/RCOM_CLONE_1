#ifndef RECEIVER_H
#define RECEIVER_H

#include "dataProtocol.h"

int openReceiver(struct linkLayer *link);


// CAN THE RECEIVER EVEN DO THIS?
// I DON'T THINK SO BECAUSE THE TRANSMITTER ISN'T GOING TO BE READING AND WAITING FOR A DISCONNECT BUT OH WELL
int closeReceiver(struct linkLayer *link);

#endif