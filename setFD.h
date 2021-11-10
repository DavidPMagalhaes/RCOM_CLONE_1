#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

int setFD(int argc, char **argv, struct termios *oldtio, struct termios *newtio)
{

  int fd;

  if ((argc < 2) ||
      ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
       (strcmp("/dev/ttyS11", argv[1]) != 0)))
  {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);

  // Makes the fd nonblocking. We'd rather achieve that by another method since this will make
  // the read to throw error 11: Resource temporarily unavailable
  // int flags = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  if (fd < 0)
  {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, oldtio) == -1)
  { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
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
    exit(-1);
  }

  printf("New termios structure set\n");

  return fd;
}

restoreFD(int fd, struct termios oldtio)
{
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
}