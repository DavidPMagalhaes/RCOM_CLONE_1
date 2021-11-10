int got_UA = FALSE;

const u_int8_t BCC = A ^ C;  //BCC=XOR(A,C)
int flag =1, conta=1;
int fd;
u_int8_t set[5] = {FLAG, A, C, BCC, FLAG};

void send_set(){
  int res;
  res = write(fd,set,sizeof(set));
  if(res==-1){
      printf("ERROR\n");
      exit(1);
  } 
}

void read_ua(){
  int i=0;
  int res;
  u_int8_t buf[255];

  while (STOP==FALSE) {       /* loop for input /
    u_int8_t byte_received;
    res = read(fd,&byte_received,1);   / returns after 5 chars have been input /
    if(res==-1){
      printf("ERROR\n");
      exit(1);
    }
    if(i==0 && byte_received !=FLAG){
      continue;
    }
    if(i>0 && byte_received == FLAG){ //last byte sent by SET
      STOP =TRUE;
    }
    buf[i++]=byte_received;               / so we can printf... */
    }

    got_UA = TRUE;

    printf("Printing UA:\n");
    for(int j=0;j<5;j++){
      printf("%u\n",buf[j]);
    }
}

void atende() // atende alarme
{
  if(!got_UA){
    send_set();
    conta++;
    printf("alarme # %d\n", conta);
    if(conta < 4){
      alarm(3);
    }
    else{
      printf("ERROR - Timeout over. It wasn't possible to receive UA successfully\n");
      exit(1);
    }
  }
}

int main(int argc, char** argv)
{
    int c;
    struct termios oldtio,newtio;
    int i, sum = 0, speed = 0;
    printf("New termios structure set\n");

    signal(SIGALRM, atende);  //instala rotina que atende interrupção


    send_set();//Send SET

    alarm(3);

    read_ua();//Receive UA

    sleep(1);
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    close(fd);
    return 0;
} 