#include "application.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    int res;
    int port;
    if (argc < 3)
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
        printf("Call with arguments <port> <filename> where <port> fullfilles <condition>\n");
    }
    start(port, argv[2], strlen(argv[2]));
}

void start(int COM, char *filename, int filenameLen)
{
    struct stat st;
    ssize_t fdSz;
    char *buf;

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        printf("Error opening file\n");
        exit(1);
    }
    fstat(fd, &st);
    off_t size = st.st_size;
    buf = malloc(size);
    fdSz = read(fd, buf, size);
    printf("%ld\n", fdSz);
    for (int i = 0; i < size; i++)
    {
        printf("%02x", buf[i]);
    }
    free(buf);
    return;
}
