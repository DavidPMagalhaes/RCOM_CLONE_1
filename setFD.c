#include "setFD.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

int setFD(struct linkLayer *link, struct termios *oldtio, struct termios *newtio)
{

    int fd;
    /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(link->port, O_RDWR | O_NOCTTY);

    // Makes the fd nonblocking. We'd rather achieve that by another method since this will make
    // the read to throw error 11: Resource temporarily unavailable
    // int flags = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    if (fd < 0)
    {
        perror(link->port);
        exit(1);
    }

    if (tcgetattr(fd, oldtio) == -1)
    { /* save current port settings */
        perror("tcgetattr");
        exit(1);
    }

    bzero(newtio, sizeof((*newtio)));
    (*newtio).c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    (*newtio).c_iflag = IGNPAR;
    (*newtio).c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    (*newtio).c_lflag = 0;

    //Currently, unblocks read after timeout with 0.1 seconds
    (*newtio).c_cc[VTIME] = 1; /* inter-character timer unused */
    (*newtio).c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if (tcsetattr(fd, TCSANOW, newtio) == -1)
    {
        perror("tcsetattr");
        exit(1);
    }

    printf("New termios structure set\n");

    link->fd = fd;
    return 0;
}

int closeFD(int fd, struct termios oldtio)
{
    if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
    {
        perror("tcsetattr");
        exit(1);
    }

    close(fd);
    return 0;
}