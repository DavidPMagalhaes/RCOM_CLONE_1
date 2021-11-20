#include "application.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "dataProtocol.h"
#include "commandMessages.h" //printFrame

int main(int argc, char **argv)
{
    int res;
    int port;
    if (argc < 2)
    {
        printf("Call with arguments <port> <filename>\n");
        exit(1);
    }
    errno = 0;
    port = strtol(argv[1], NULL, 10);
    if (errno != 0)
    {
        // The first argument probably wasn't a number
        printf("Call with arguments <port> <filename>\n");
        exit(1);
    }
    if (!(1 || port))
    {
        // Port does not fulfill certain condition
        printf("Call with arguments <port> <filename> where <port> fulfills <condition>\n");
    }
    if (argc == 2)
    {
        startReceiverProtocol(port);
    }
    else if (argc == 3)
    {
        startTransmitterProtocol(port, argv[2], strlen(argv[2]));
    }
    else
    {
        printf("Call with arguments <port> <filename>\n");
        exit(1);
    }
}

void startTransmitterProtocol(int port, char *filename, int filenameLen)
{
    ssize_t size;
    u_int8_t *buf;
    struct stat st;
    off_t fileSize;
    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fstat(fd, &st);
    fileSize = st.st_size;
    buf = malloc(fileSize);
    size = read(fd, buf, fileSize);
    printf("Im here %ld", size);
    printf("\n");
    sendFile(port, buf, size, filename);
    free(buf);
    return;
}

void startReceiverProtocol(int port)
{
    int res = 0;
    u_int8_t *buf;
    char *filename;
    ssize_t size;
    res = readFile(port, buf, &size, filename);
    if (res != 0)
    {
        // Error
        exit(1);
    }
    // TODO
    // Add a prefix to the filename
    res = writeFile(filename, buf, size);
    if (res != 0)
    {
        // Error
        exit(1);
    }
    free(buf);
    free(filename);
}

void sendFile(int port, u_int8_t *buf, ssize_t size, char *filename)
{
    int res;
    linkType linkType = TRANSMITTER;
    int fd = llopen(port, linkType);
    if (fd == -1)
    {
        printf("Transmitter: Error establishing connection\n");
        exit(1);
    }
    writeFrames(fd, buf, size, filename);

    res = llclose(fd);
    if(res != 0){
        printf("Transmitter: Couldn't inform receiver to close connection. Manual action may be necessary\n");
    }

}

int readFile(int port, u_int8_t *buf, ssize_t *size, char *filename)
{
    // Don't forget to allocate space for filename
    linkType linkType = RECEIVER;
    int fd = llopen(port, linkType);
    if (fd == -1)
    {
        printf("Receiver: Error establishing connection\n");
        exit(1);
    }

    readFrames(fd, buf, size, filename);
    return 0;
}

int writeFile(char *filename, u_int8_t *buf, ssize_t size)
{
    int res;
    int fd = open(filename, O_WRONLY | O_TRUNC);
    res = write(fd, buf, size);
    if (res == -1)
    {
        printf("Receiver: Error writing to file\n");
        exit(1);
    }
    writeFrames(fd, buf, size, filename);
    return 0;
}

void writeFrames(int fd, u_int8_t *buf, ssize_t size, char *filename)
{

    // What if the filename is 255 bytes long. Then the control frame won't have enoug bytes.
    // The data packets depends on the buffer size of the link layer, though.
    // So, should I make the link layer have a buffer long enough for a 255 bytes filename plus the others parameters?
    // What if I had more parameters. Would I change the link frame size then
    // Ask the teacher what I should do in this case.

    u_int8_t *controlBuf;
    int controlSize;
    assembleControlFrame(&controlBuf, &controlSize, size, filename);

    // Start control frame
    controlBuf[0] = 2; // For start control
    writeFrame(fd, controlBuf, controlSize);

    // Assemble information and send it
    writeInformationFrames(fd, buf, size);

    // End control frame
    controlBuf[0] = 3; // For end control
    writeFrame(fd, controlBuf, controlSize);
    free(controlBuf);
}

void writeFrame(int fd, u_int8_t *buf, int size)
{
    // We assume size is enough to fit in the frame buffer
    int res = llwrite(fd, buf, size);
    if (res == -1)
    {
        // Time out
        printf("Transmitter: The receiver timed out\n");
        exit(1);
    }
    if (res != size)
    {
        // Different number of bytes was written for some reason
        printf("Transmitter: Couldn't write full packet\n");
        exit(1);
    }
}

void writeInformationFrames(int fd, u_int8_t *buf, ssize_t size)
{
    u_int8_t frameBuf[LINK_LAYER_BUFFER_SIZE];
    u_int16_t datasize = LINK_LAYER_BUFFER_SIZE - INFORMATION_PACKET_HEAD_SIZE;
    u_int16_t packetDataSize;
    u_int8_t packetSeq = 0;
    ssize_t bufIndex = 0;
    int i =0;
    while (bufIndex < size)
    {
        if (bufIndex + datasize > size)
        {
            packetDataSize = size - bufIndex;
        }
        else
        {
            packetDataSize = datasize;
        }

        assembleInformationFrame(buf, bufIndex, frameBuf, packetDataSize, packetSeq);
        writeFrame(fd, frameBuf, INFORMATION_PACKET_HEAD_SIZE + packetDataSize);
        // Index is mod 255 or 256. Should be 256 since that would only go up to 255 in a byte
        packetSeq = (packetSeq + 1) % 256;
        bufIndex += packetDataSize;
        i++;
    }
    printf("No print %d", i);

    // while (size > 0)
    // {
    //     // Is this comparison dangerous because of the types?
    //     // https://stackoverflow.com/a/6636818 Should be fine: u_int16_t is converted to ssize_t
    //     if (size < datasize)
    //     {
    //         packetSize = size;
    //     }
    //     else
    //     {
    //         packetSize = datasize;
    //     }

    //     // Index is mod 255 or 256. Should be 256 since that would only go up to 255 in a byte
    //     packetSeq = (packetSeq + 1) % 256;
    //     size -= packetSize;
    // }
}

void assembleControlFrame(u_int8_t **buf, int *size, ssize_t fileSize, char *filename)

{
    // Should I do the successive parameters with an accumulator for the index instead. For more than 2 parameters should work better

    (*size) = 1 + 2 + sizeof(fileSize) + 2 + strlen(filename) + 1;
    (*buf) = malloc((*size));               //Will be freed in the caller functoin
    (*buf)[0] = (u_int8_t)0;                // Control field
    (*buf)[1] = (u_int8_t)0;                // Indicating file size
    (*buf)[2] = (u_int8_t)sizeof(fileSize); // Indicating how many bytes for file size

    // https://stackoverflow.com/a/40828860
    // little endian representation
    memcpy((*buf + 3), (u_int8_t *)&fileSize, sizeof(fileSize)); // File size

    (*buf)[3 + sizeof(fileSize)] = (u_int8_t)1;                                        // Indicating file name
    (*buf)[3 + sizeof(fileSize) + 1] = (u_int8_t)strlen(filename);                     // Indicating how many bytes for filename
    snprintf((*buf) + 3 + sizeof(fileSize) + 2, strlen(filename) + 1, "%s", filename); //Filename
}

void assembleInformationFrame(u_int8_t *buf, ssize_t bufIndex, u_int8_t *frameBuf, u_int16_t datasize, u_int8_t seq)
{
    frameBuf[0] = 1;
    frameBuf[1] = seq;
    memcpy((frameBuf + 2), (u_int8_t *)&datasize, 2);
    memcpy((frameBuf + 4), (buf + bufIndex), datasize);
    // datasize is not the size of the frameBuf. Assembling is guaranteed not to exceed framebuf size
}

void readFrames(int fd, u_int8_t *buf, ssize_t *size, char *filename)
{
}
