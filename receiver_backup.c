/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include "commandMessages.h"
#include "setFD.h"

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define A 0x01
#define F 0x7e
#define SET 0x03
#define DISC 0x0B
#define UA 0x07
#define RR 0x05
#define REJ 0x01

volatile int STOP = FALSE;

int main(int argc, char **argv)
{
  // int fd, c, res;
  // struct termios oldtio, newtio;
  // char buf[255];

  // if ((argc < 2) ||
  //     ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
  //      (strcmp("/dev/ttyS11", argv[1]) != 0)))
  // {
  //   printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS11\n");
  //   exit(1);
  // }

  // /*
  //   Open serial port device for reading and writing and not as controlling tty
  //   because we don't want to get killed if linenoise sends CTRL-C.
  // */

  // fd = open(argv[1], O_RDWR | O_NOCTTY);
  // if (fd < 0)
  // {
  //   perror(argv[1]);
  //   exit(-1);
  // }

  // if (tcgetattr(fd, &oldtio) == -1)
  // { /* save current port settings */
  //   perror("tcgetattr");
  //   exit(-1);
  // }

  // bzero(&newtio, sizeof(newtio));
  // newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  // newtio.c_iflag = IGNPAR;
  // newtio.c_oflag = 0;

  // /* set input mode (non-canonical, no echo,...) */
  // newtio.c_lflag = 0;

  // newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  // newtio.c_cc[VMIN] = 5;  /* blocking read until 5 chars received */

  // /*
  //   VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
  //   leitura do(s) pr�ximo(s) caracter(es)
  // */

  // tcflush(fd, TCIOFLUSH);

  // if (tcsetattr(fd, TCSANOW, &newtio) == -1)
  // {
  //   perror("tcsetattr");
  //   exit(-1);
  // }

  // printf("New termios structure set\n");

  int res, fd;
  char buf[255];
  struct termios oldtio, newtio;
  fd = setFD(argc, argv, &oldtio, &newtio);

  /* loop for input */
  int i = 0;
  int flag = 0;
  while (STOP == FALSE)
  { /* loop for input */
    char c;
    res = read(fd, &c, 1); /* returns after 5 chars have been input */
    if(res == 0){
      printf("EOF\n");
    }
    if (res == -1)
    {
      printf("ERROR\n");
      exit(1);
    }
    buf[i++] = c; /* so we can printf... */
    printf("%02x", c);
    printf("Message: %s:%d\n", buf, strlen(buf));

    if (c == F)
    {
      if (!flag)
      {
        printf("Started receiving message\n");
        flag = 1;
      }
      else
      {
        //ended the message, send ack
        printf("Received message:");
        printCommand(buf);
        UAMessage(buf);
        printCommand(buf);
        res = write(fd, buf, CMDSZ);
        if (res == -1)
        {
          printf("ERROR\n");
          exit(1);
        }
        flag=0;
        continue;
      }
    }
  }
  printf("Message: %s\n", buf);

  // res = read(fd, buf, CMDSZ); /* returns after 5 chars have been input */
  // printCommand(buf);
  // printf("%s:%d", buf, res);
  // printf("%x", buf, strlen(buf));
  // fflush(stdout);
  // buf[res] = '\0'; /* so we can printf... */
  // printf(":%s:%d\n", buf, res);
  // if (buf[0] == 'z')
  //   STOP = TRUE;

  // while (STOP == FALSE)
  // {                             /* loop for input */
  //     res = read(fd, buf, 255); /* returns after 5 chars have been input */
  //     printf("Read");
  //     buf[res] = '\0'; /* so we can printf... */
  //     printf(":%s:%d\n", buf, res);

  //     if (dataIsViable(buf))
  //     {
  //         printf("Data was received correctly\n");
  //         UAMessage(buf);
  //         int r = write(fd, buf, res);
  //     }
  //     if (buf[0] == 'z')
  //     {
  //         STOP = TRUE;
  //         printf("Interrupting program\n");
  //     }
  // }

  /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */

  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
