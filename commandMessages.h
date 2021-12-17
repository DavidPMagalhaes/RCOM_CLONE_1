#ifndef COMMANDMESSAGES_H
#define COMMANDMESSAGES_H

#include <stdio.h>
#include <sys/types.h>

//COMMANDS
#define A_EM 0x03  //commands by the emissor (or responses by the receiver)
#define A_REC 0x01 //commands by the receiver (or responses by the emissor)
#define F 0x7e
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR 0x05
#define RR_N1 0x85
#define REJ 0x01
#define REJ_N1 0x81
#define CMDSZ 5 //[FLAG,ADDRESS,CMD,BCC,FLAG]

void printCommand(u_int8_t buf[]);

void printFrame(u_int8_t *buf, int bufLen);

void commandMessage(u_int8_t buf[], u_int8_t A, u_int8_t C);

void SETMessage(u_int8_t buf[]);

void DISCMessage(u_int8_t buf[], u_int8_t A);

void UAMessage(u_int8_t buf[], u_int8_t A);

void RRMessage(u_int8_t buf[], int R);

void REJMessage(u_int8_t buf[], int R);

void IMessage (u_int8_t buf[], int S);

#endif //COMMANDMESSAGES_H