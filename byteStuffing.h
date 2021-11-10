#ifndef BYTESTUFFING_H
#define BYTESTUFFING_H
#include "dataProtocol.h"
#include "commandMessages.h" // Instead of #define F 0x7e

#define ESCAPE 0x7d
#define ESCAPE_THE_FLAG 0x5e
#define ESCAPE_THE_ESCAPE 0x5d
#define FIRST_DATA_INDEX 4

#define SPACE_ALLOC_SIZE 20

//Stuffing and destuffing already take care of the protection byte

void stuff(struct frame *frame, char tostuff[], int tostuffSize, char protectionByte)

void destuff(struct frame *frame, char *buffer, int bufferCapacity, int *bufferLength);

void allocSpace(struct frame *frame, int space);

int allocBufferSpace(char *buffer, int prevSpace, int extraSpace);

#endif //BYTESTUFFING_H