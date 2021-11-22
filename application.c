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
        // The first argument probably wasn't a number
        printf("Call with arguments <port> <filename> [...options] \n");
        exit(1);
    }
    if (!(1 || port))
    {
        // Port does not fulfill certain condition
        printf("Call with arguments <port> <filename> [...options]  where <port> fulfills <condition>\n");
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
        // There was two arguments and the rest was flags or reached the end of the arguments
        startTransmitterProtocol(port, argv[2], strlen(argv[2]));
    }
    else
    {
        // There was more than two arguments before finding a flag or reaching the end of the arguments
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
    fstat(fd, &st);
    fileSize = st.st_size;
    buf = malloc(fileSize);
    size = read(fd, buf, fileSize);
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
    res = readFile(port, &buf, &size, &filename);
    if (res != 0)
    {
        // Error
        exit(1);
    }
    // TODO
    // Add a prefix to the filename
    res = saveFile(filename, buf, size);
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
    if (res != 0)
    {
        printf("Transmitter: Couldn't inform receiver to close connection. Manual action may be necessary\n");
    }
}

void writeFrames(int fd, u_int8_t *buf, ssize_t size, char *filename)
{

    // What if the filename is 255 bytes long. Then the control frame won't have enoug bytes.
    // The data packets depends on the buffer size of the link layer, though.
    // So, should jmake the link layer have a buffer long enough for a 255 bytes filename plus the others parameters?
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

void writeInformationFrames(int fd, u_int8_t *buf, ssize_t size)
{
    u_int8_t frameBuf[LINK_LAYER_BUFFER_SIZE];
    u_int16_t datasize = LINK_LAYER_BUFFER_SIZE - INFORMATION_PACKET_HEAD_SIZE;
    u_int16_t packetDataSize;
    u_int8_t packetSeq = 0;
    ssize_t bufIndex = 0;
    int i = 0;
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

int readFile(int port, u_int8_t **buf, ssize_t *size, char **filename)
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

int saveFile(char *filename, u_int8_t *buf, ssize_t size)
{
    char *filename2;
    int res;
    int filename2size = strlen(filename) + 4 + 1;
    filename2 = malloc(filename2size);
    sprintf(filename2, "DATA%s", filename);

    int fd = open(filename2, O_WRONLY | O_TRUNC | O_CREAT);
    if (fd == -1)
    {
        // printf("%d", errno);
        printf("Receiver: Unable to open output file\n");
    }
    // free(filename2);
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
                // Something very wrong happened
                printf("Receiver: Unexpected behaviour detected. Exiting\n");
                exit(1);
            }
        }

        control = frameBuf[0];
        switch (control)
        {
        case 1:
            if (expecting_end)
            {
                // Too many safety measures?
                printf("Receiver: No more information packets expected\n");
                exit(1);
            }

            // Information packet
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
            (*buf) = malloc(*size);
            memcpy(frameBufStart, frameBuf, controlPacketSize);
            start = 1;
            break;
        case 3:
            // End packet
            if (!start)
            {
                // Something very wrong happened
                printf("Receiver: Unexpected behaviour detected. Exiting\n");
                exit(1);
            }
            if (!expecting_end)
            {
                // Too many safety measures?

                // This might actually be a feasible scenario where the last information packet is delayed. What should I do in this case.
                // We should have a windowBuf[256][255] to implement the sliding window mechanism perhaps
                // Talk to the teacher TODO
                printf("Receiver: Not expecting end packet\n");
                exit(1);
            }
            // Compare frameBuf to frameBufStart to see if the information is the same
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
    int i = 1;
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
    // Byte 0 is already verified to be 1
    // How to deal with wrong sequence numbers?
    // Do I need to put them in the right place and the lowest received till now is my bufIndex?
    u_int8_t seqNo = frameBuf[1];
    u_int16_t datasize;
    memcpy(&datasize, frameBuf + 2, 2);
    if (seqNo == (*seq))
    {
        // We are getting the subsequent data and there are no problems
        (*seq) = ((*seq) + 1) % 256;
        memcpy(buf + (*bufIndex), frameBuf + INFORMATION_PACKET_HEAD_SIZE, datasize);
        (*bufIndex) += datasize;
    }
    else
    {
        // TODO
        // See with the teacher what is the right approach here!!!
        // Works as a sort of sliding window. Need to put the bytes in the right places
        printf("Receiver: Incorrect data packet sequence order. Ask the developer to implement safety mechanisms\n");
        exit(1);
    }
    return;
}