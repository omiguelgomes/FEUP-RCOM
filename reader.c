#include "reader.h"
#include "dataLink.c"

volatile int STOP_BOOL=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char *buf = malloc(MAX_SIZE*sizeof(char));

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS10\n");
      exit(1);
    }

    //parsing argv[1]
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
      printf("Port number must be {0, 1, 10, 11}\n");
      exit(1);
    }
    llopen(app.fd, RECEIVER);

    //condition to be replaced to alarm
    while(TRUE)
    {
      if(llread(app.fd, buf) == 1)
        break;
    }

    /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */

    tcsetattr(app.fd,TCSANOW,&oldtio);
    close(app.fd);
    return 0;
}

