#include "dataLink.c"

volatile int END=FALSE;

int fd,c, res;
struct termios oldtio,newtio;
applicationLayer app;
linkLayer ll;

int main(int argc, char** argv)
{

    char buf[255];
    int i, sum = 0, speed = 0;

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS10\n");
      exit(1);
    }

    char *s = argv[1];
    int n = 9;
    char *s2 = s + n;
    while (*s2)
    {
      *s = *s2;
      ++s;
      ++s2;
   }
   *s = '\0';

    app.fd = atoi(argv[1]);

    if(app.fd != 0 && app.fd != 1 && app.fd != 10 && app.fd != 11){
      printf("Port number must be {0, 1, 10, 11}");
      exit(1);
    }

    //default open, no flags
    llopen(app.fd, TRANSMITER);
    printf("Enter a line to be transmitted: \n");


    fgets(buf, sizeof(buf), stdin);

    int len = strlen(buf);
    
    res = write(fd,buf,len);
    printf("%d bytes written\n", res);
 
    printf("waiting for confirmation...\n");

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

    while (END==FALSE) {
      res = read(fd,buf,255);
      buf[res]='\0';
      printf("%s", buf);
      if (buf[res]=='\0') END=TRUE;
    }

    printf("Confirmation received!\n");

   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    
    return llclose(fd);
}
