#ifndef SETFD_H
#define SETFD_H

#include <termios.h>
#include "dataProtocol.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int setFD(struct linkLayer *link, struct termios *oldtio, struct termios *newtio);

int closeFD(int fd, struct termios oldtio);

#endif