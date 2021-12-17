#ifndef DATA_PROTECTION_H
#define DATA_PROTECTION_H

#include <sys/types.h>

u_int8_t createProtectionByte(u_int8_t *buffer, int length);

int verifyProtectionByte(u_int8_t *buffer, int length);

#endif //DATA_PROTECTION_H