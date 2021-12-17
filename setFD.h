#ifndef SETFD_H
#define SETFD_H

#include <termios.h>
#include "dataProtocol.h"

// Change according to Baudrate to be used
// #define BAUDRATE B4800
// #define BAUDRATE B9600
// #define BAUDRATE B19200
#define BAUDRATE B38400
// #define BAUDRATE B57600
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int setFD(struct linkLayer *link, struct termios *oldtio, struct termios *newtio);

int closeFD(int fd, struct termios oldtio);

#endif //SETFD_H