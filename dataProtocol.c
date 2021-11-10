#include "dataProtocol.h"

void initializeFrame(struct frame *frame)
{
    char *newFrame = (char *)malloc(MAX_SIZE);
    frame->stuffedFrame = newFrame;
    frame->stuffedFrameSize = MAX_SIZE;
}

int llopen(int porta, int soup){
    char* porta2 = malloc(sizeof(char) * (strlen(COM) + 1);
    sprintf(porta2,%s%d,COM,porta);
    int fd = setfd(porta);
    switch(type) {
        case SENDER:
            if (senderSomething(fd) > 0) return fd; //emissor.c
        case RECEiVER:
            if (receiverSomething(fd) > 0) return fd; //recetor.c
            
    }
    return -1;
}

void llwrite(){

}

void llread(){

}

int llclose(int fd){
    

    return -1;
}
