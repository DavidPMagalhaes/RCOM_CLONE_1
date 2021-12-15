#include "dataProtection.h"

u_int8_t createProtectionByte(u_int8_t *buffer, int length){
    u_int8_t byte = 0;
    for(int i = 0; i < length; i++){
        byte ^= buffer[i];
    }
    return byte;
}

int verifyProtectionByte(u_int8_t *buffer, int length){
    u_int8_t byte = 0;
    for(int i = 0; i< length; i++){
        byte ^= buffer[i];
    }
    return (byte == 0);
}