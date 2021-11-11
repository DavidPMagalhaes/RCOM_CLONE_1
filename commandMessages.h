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
#define REJ 0x01
#define CMDSZ 5 //[FLAG,ADDRESS,CMD,BCC,FLAG]

int dataIsViable(char mes[], int size);

void printCommand(char buf[]);

void printFrame(char *buf, int bufLen);

void commandMessage(char buf[], char A, char C);

void SETMessage(char buf[]);

void DISCMessage(char buf[], char A);

void UAMessage(char buf[], char A);

void RRMessage(char buf[], int R);

void REJMessage(char buf[], int R);

#endif