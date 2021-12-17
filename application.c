#include "application.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "dataProtocol.h"
#include "commandMessages.h" //printFrame
#include "options.h"

int main(int argc, char **argv)
{
    int res;
    int port;
    int argNo;
    time_t seed = time(NULL);
    CREATE_OPTIONS(argc, argv, seed);
    if (argc < 2)
    {
        printf("Call with arguments <port> <filename> [...options] \n");
        exit(1);
    }
    errno = 0;
    port = strtol(argv[1], NULL, 10);
    if (errno != 0)
    {
        // The first argument wasn't a number
        printf("Call with arguments <port> <filename> [...options] \n");
        exit(1);
    }

    for (argNo = 2; argNo < argc; argNo++)
    {
        // Starts in 2 because we already got the port. Only need the files or options now
        if (OPTION_IS_FLAG(argv[argNo]))
        {
            break;
        }
    }

    if (argNo == 2)
    {
        // There was only one argument and the rest was flags or reached the end of the arguments
        startReceiverProtocol(port);
    }
    else if (argNo == 3)
    {
        // There were two arguments and the rest was flags or reached the end of the arguments
        startTransmitterProtocol(port, argv[2], strlen(argv[2]));
    }
    else
    {
        // There were more than two arguments before finding a flag or reaching the end of the arguments
        printf("Call with arguments <port> <filename> [...options] \n");
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

    // Get file size and allocate buffer space
    fstat(fd, &st);
    fileSize = st.st_size;
    buf = malloc(fileSize);

    // Read file
    size = read(fd, buf, fileSize);

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
    printf("Starting receiver protocol\n");
    res = readFile(port, &buf, &size, &filename);
    if (res != 0)
    {
        exit(1);
    }

    printf("Saving file to memory\n");
    res = saveFile(filename, buf, size);
    if (res != 0)
    {
        exit(1);
    }
    free(buf);
    free(filename);
    printf("File saved\n");
}

void sendFile(int port, u_int8_t *buf, ssize_t size, char *filename)
{
    printf("Starting transmitter protocol\n");
    int res;
    linkType linkType = TRANSMITTER;
    int fd = llopen(port, linkType);
    if (fd == -1)
    {
        printf("Transmitter: Error establishing connection\n");
        exit(1);
    }

    printf("Connection established\n");
    writeFrames(fd, buf, size, filename);
    printf("Data sent. Closing connection\n");
    res = llclose(fd);
    if (res != 0)
    {
        printf("Transmitter: Couldn't inform receiver to close connection. Manual action may be necessary\n");
    }
    printf("Connection closed\n");
}

void writeFrames(int fd, u_int8_t *buf, ssize_t size, char *filename)
{
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

void writeInformationFrames(int fd, u_int8_t *buf, ssize_t size)
{
    // Writes all of the information frames
    u_int8_t frameBuf[MAX_SIZE];
    u_int16_t datasize = MAX_SIZE - INFORMATION_PACKET_HEAD_SIZE;
    u_int16_t packetDataSize;
    u_int8_t packetSeq = 0;
    ssize_t bufIndex = 0;
    int i = 0;
    while (bufIndex < size)
    {
        // Calculates how many bites needs to send
        if (bufIndex + datasize > size) // Doesn't need to use the whole frame to send the information
        {
            packetDataSize = size - bufIndex;
        }
        else // Will use all the frame available
        {
            packetDataSize = datasize;
        }

        assembleInformationFrame(buf, bufIndex, frameBuf, packetDataSize, packetSeq);
        writeFrame(fd, frameBuf, INFORMATION_PACKET_HEAD_SIZE + packetDataSize);
        packetSeq = (packetSeq + 1) % 256;
        bufIndex += packetDataSize;
        i++;
    }
}

void assembleControlFrame(u_int8_t **buf, int *size, ssize_t fileSize, char *filename)

{
    (*size) = 1 + 2 + sizeof(fileSize) + 2 + strlen(filename) + 1;
    (*buf) = malloc((*size));               //Will be freed in the caller function
    (*buf)[0] = (u_int8_t)0;                // Control field
    (*buf)[1] = (u_int8_t)0;                // Indicating file size
    (*buf)[2] = (u_int8_t)sizeof(fileSize); // Indicating how many bytes for file size
    memcpy((*buf + 3), (u_int8_t *)&fileSize, sizeof(fileSize)); // File size

    (*buf)[3 + sizeof(fileSize)] = (u_int8_t)1;                                        // Indicating file name
    (*buf)[3 + sizeof(fileSize) + 1] = (u_int8_t)strlen(filename);                     // Indicating how many bytes for filename
    snprintf((*buf) + 3 + sizeof(fileSize) + 2, strlen(filename) + 1, "%s", filename); // Filename
}

void assembleInformationFrame(u_int8_t *buf, ssize_t bufIndex, u_int8_t *frameBuf, u_int16_t datasize, u_int8_t seq)
{
    frameBuf[0] = 1;
    frameBuf[1] = seq;
    memcpy((frameBuf + 2), (u_int8_t *)&datasize, 2);
    memcpy((frameBuf + 4), (buf + bufIndex), datasize);
    // datasize is not the size of the frameBuf but of the data. Assembling is guaranteed not to exceed framebuf size
}

void writeFrame(int fd, u_int8_t *buf, int size)
{
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

int readFile(int port, u_int8_t **buf, ssize_t *size, char **filename)
{
    // Don't forget to allocate space for filename
    linkType linkType = RECEIVER;
    printf("Waiting for connection to be established\n");
    int fd = llopen(port, linkType);
    if (fd == -1)
    {
        printf("Receiver: Error establishing connection\n");
        exit(1);
    }
    printf("Connection established\n");
    readFrames(fd, buf, size, filename);
    printf("File received\n");
    return 0;
}

int saveFile(char *filename, u_int8_t *buf, ssize_t size)
{
    char *filename2;
    int res;

    // Write a prefix "DATA" to the filename
    int filename2size = strlen(filename) + 4 + 1;
    filename2 = malloc(filename2size);
    sprintf(filename2, "DATA%s", filename);

    int fd = open(filename2, O_WRONLY | O_TRUNC | O_CREAT);
    if (fd == -1)
    {
        printf("Receiver: Unable to open output file. Make sure file doesn't exist already\n");
    }
    free(filename2);

    res = write(fd, buf, size);
    if (res == -1)
    {
        printf("Receiver: Error writing to file\n");
        exit(1);
    }
    return 0;
}

void readFrames(int fd, u_int8_t **buf, ssize_t *size, char **filename)
{
    int res;
    int start = 0, end = 0, expecting_end = 0;
    u_int8_t control;
    u_int8_t frameBuf[LINK_LAYER_BUFFER_SIZE], frameBufStart[LINK_LAYER_BUFFER_SIZE];
    int controlPacketSize = 0;
    ssize_t bufIndex = 0;
    u_int8_t seq = 0;

    while (1)
    {
        res = llread(fd, frameBuf);
        if (res == 0)
        {
            if (start && !end)
            {
                // We received a disconnection message without having received the full file
                printf("Receiver: Disconnection before operation concluded\n");
                exit(1);
            }
            else if (start && end)
            {
                // Disconnect request successfully received
                return;
            }
            else if (!start && !end)
            {
                printf("Receiver: No file received. Exiting\n");
                return;
            }
            else if (!start && end)
            {
                // We received the end packet before the start packet
                printf("Receiver: Unexpected behaviour detected. Exiting\n");
                exit(1);
            }
        }
        control = frameBuf[0];
        switch (control)
        {
        case 1:
            // Information packet
            if (expecting_end)
            {
                printf("Receiver: No more information packets expected\n");
                exit(1);
            }

            readInformationFrame(*buf, &bufIndex, frameBuf, &seq);
            if (bufIndex == *size)
            {
                expecting_end = 1;
            }
            break;
        case 2:
            // Start packet
            if (expecting_end)
            {
                // Too many safety measures?
                printf("Receiver: No more information packets expected\n");
                exit(1);
            }
            if (start)
            {
                printf("Receiver: Restarting package reception\n");
                end = 0;
                bufIndex = 0;
                expecting_end = 0;
                seq = 0;
            }
            controlPacketSize = readControlFrame(frameBuf, filename, size);
            (*buf) = malloc(*size); // Alloc space for the file in the buffer
            memcpy(frameBufStart, frameBuf, controlPacketSize); // Keep a copy of the start packet
            start = 1;
            break;
        case 3:
            // End packet
            if (!start)
            {
                // Received end packet without receiving start packet
                printf("Receiver: Unexpected behaviour detected. Exiting\n");
                exit(1);
            }
            if (!expecting_end)
            {
                // Received end packet without reading the indicated number of bytes
                printf("Receiver: Not expecting end packet\n");
                exit(1);
            }
            // Compare the start and end packets to verify if the information is the same
            frameBuf[0] = 0;
            frameBufStart[0] = 0;
            if (memcmp(frameBuf, frameBufStart, controlPacketSize) != 0)
            {
                printf("Receiver: Start and end control packet are not matching\n");
                exit(1);
            }
            end = 1;
            break;

        default:
            printf("Receiver: Invalid control packet %02x\n", control);
            exit(1);
        }
    }
}

int readControlFrame(u_int8_t *buf, char **filename, ssize_t *filesize)
{
    // Byte 0 is already verified to be 2 or 3
    int i = 1; //Control frame iterator
    u_int8_t param, bytes;

    for (int j = 0; j < NUM_CONTROL_PARAMS; j++)
    {
        param = buf[i];
        i++;
        bytes = buf[i];
        i++;
        switch (param)
        {
        case 0:
            // fileSize
            memcpy(filesize, (buf + i), bytes);
            break;
        case 1:
            // filename
            (*filename) = malloc(bytes);
            memcpy(*filename, (buf + i), bytes);
            break;
        default:
            break;
        }
        i += bytes;
    }
    return i;
}

void readInformationFrame(u_int8_t *buf, ssize_t *bufIndex, u_int8_t *frameBuf, u_int8_t *seq)
{
    u_int8_t seqNo = frameBuf[1];
    u_int16_t datasize;
    memcpy(&datasize, frameBuf + 2, 2); // Number of data bytes in the packet
    if (seqNo == (*seq))
    {
        // Correct sequence number for the packet
        (*seq) = ((*seq) + 1) % 256;
        memcpy(buf + (*bufIndex), frameBuf + INFORMATION_PACKET_HEAD_SIZE, datasize); //Copy data
        (*bufIndex) += datasize;
    }
    else
    {
        // Wrong packet sequence number
        printf("Receiver: Incorrect data packet sequence order.\n");
        exit(1);
    }
    return;
}