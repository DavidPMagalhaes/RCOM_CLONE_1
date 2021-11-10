/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>

#include "commandMessages.h"
#include "setFD.h"

#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int flag = 0, count = 1;

void atende() // atende alarme
{
  printf("Alarme #%d\n", count);
  flag = 1;
  count++;
}

void resetatende()
{
  printf("Alarm resetted\n");
  (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao
}
void resetalarm()
{
  printf("Reseting alarm\n");
  (void)signal(SIGALRM, resetatende); // instala  rotina que atende interrupcao
}

int main(int argc, char **argv)
{
  int res, fd;
  int mesFlag = 0;
  int i = 0;
  char c;
  u_int8_t buf[255];
  u_int8_t mes[255];
  struct termios oldtio, newtio;
  fd = setFD(argc, argv, &oldtio, &newtio);
  (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao

  SETMessage(mes);
  printCommand(mes);

  //Write the SET message
  printf("Setting connection\n");
  flag = 1; // Send a message will now wait for timeout
  while (STOP == FALSE && count < 4)
  {
    if (flag)
    {
      res = write(fd, mes, CMDSZ);
      if (res == -1)
      {
        printf("ERROR\n");
        exit(1);
      }
      printf("Setting alarm\n");
      alarm(3);
      flag = 0;
    }

    res = read(fd, &c, 1); /* returns after 5 chars have been input */
    if (res == 0)
    {
      continue;
    }
    if (res == -1)
    {
      printf("ERROR");
      exit(1);
    }
    buf[i++] = c; /* so we can printf... */
    printf("%02x", c);
    printf("Message: %s:%d\n", buf, strlen(buf));

    if (c == F)
    {
      if (!mesFlag)
      {
        printf("Started receiving message\n");
        mesFlag = 1;
      }
      else
      {
        //ended the message, send ack
        printf("Received message:");
        printCommand(buf);
        
        resetalarm();
        mesFlag = 0;
        i = 0;
        continue;
      }
    }
  }
  printf("Ended transmission phase\n");

  restoreFD(fd, oldtio);

  return 0;
}
