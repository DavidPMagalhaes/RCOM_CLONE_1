#ifndef BYTESTUFFING_H
#define BYTESTUFFING_H
#include "dataProtocol.h"
#include "commandMessages.h" // Instead of #define F 0x7e

#define ESCAPE 0x7d
#define ESCAPE_THE_FLAG 0x5e
#define ESCAPE_THE_ESCAPE 0x5d
#define FIRST_DATA_INDEX 4

#define SPACE_ALLOC_SIZE 20

int stuff(struct frame *frame, u_int8_t tostuff[], int tostuffSize,u_int8_t protectionByte);

int destuff(struct frame *frame, u_int8_t *buffer);

#endif //BYTESTUFFING_H