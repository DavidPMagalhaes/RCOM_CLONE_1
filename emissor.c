/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "commandMessages.h"
#include "setFD.h"
#include <string.h>

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int main(int argc, char **argv)
{
  int res, fd;
  char buf[255];
  char mes[255];
  struct termios oldtio, newtio;
  fd = setFD(argc, argv, &oldtio, &newtio);

  // for (int i = 0; i < 255; i++)
  // {
  //   buf[i] = 'a';
  // }

  // /*testing*/
  // buf[25] = '\n';

  // res = write(fd, buf, 255);
  // printf("%d bytes written\n", res);

  SETMessage(mes);
  printCommand(mes);

  //Write the SET message
  printf("Setting connection\n");
  res = write(fd, mes, CMDSZ);
  if (res == -1)
  {
    printf("ERROR\n");
    exit(1);
  }
  int i = 0;
  int flag = 0;
  while (STOP == FALSE)
  { /* loop for input */
    char c;
    res = read(fd, &c, 1); /* returns after 5 chars have been input */
    if (res == 0)
    {
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
        
        continue;
      }
    }
  }

  // fflush(stdout);
  // res = read(fd, buf, 255); /* returns after 5 chars have been input */
  // printf("%ld", res);
  // buf[res] = '\0'; /* so we can printf... */

  // printf("Message:");
  // printf(":%sSize:%ld\n", buf, res);

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1)
  {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}
