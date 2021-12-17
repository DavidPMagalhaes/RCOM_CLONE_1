#ifndef APPLICATION_H
#define APPLICATION_H

#include <sys/types.h>

#define LINK_LAYER_BUFFER_SIZE 255 //Needs to be the same as the MAX_SIZE in dataProtocol.h
#define INFORMATION_PACKET_HEAD_SIZE 4
#define NUM_CONTROL_PARAMS 2 // Filesize and filename

int main();

void startTransmitterProtocol(int port, char *filename, int filenameLen);

void startReceiverProtocol(int port);

void sendFile(int port, u_int8_t *buf, ssize_t size, char *filename);

void writeFrames(int fd, u_int8_t *buf, ssize_t size, char *filename);

void writeInformationFrames(int fd, u_int8_t *buf, ssize_t size);

void assembleControlFrame(u_int8_t **buf, int *size, ssize_t fileSize, char *filename);

void assembleInformationFrame(u_int8_t *buf, ssize_t bufIndex, u_int8_t *frameBuf, u_int16_t datasize, u_int8_t seq);

void writeFrame(int fd, u_int8_t *buf, int size);

int readFile(int port, u_int8_t **buf, ssize_t *size, char **filename);

int saveFile(char *filename, u_int8_t *buf, ssize_t size);

void readFrames(int fd, u_int8_t **buf, ssize_t *size, char **filename);

int readControlFrame(u_int8_t *buf, char **filename, ssize_t *filesize);

void readInformationFrame(u_int8_t *buf, ssize_t *bufIndex, u_int8_t *frameBuf, u_int8_t *seq);

#endif //APPLICATION_H