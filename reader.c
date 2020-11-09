#include "reader.h"

volatile int STOP_BOOL=FALSE;

int fd,c, res;
struct termios oldtio,newtio;
applicationLayer app;
linkLayer ll;

int main(int argc, char** argv)
{
    char buf[255];
    char result[MAX_SIZE];
    int i, sum = 0, speed = 0;

    if (argc != 2) {
      printf("Usage:\n./reader port_number\n");
      exit(1);
    }


    app.fd = atoi(argv[1]);

    if(app.fd != 0 && app.fd != 1 && app.fd != 10 && app.fd != 11){
      printf("Port number must be {0, 1, 10, 11}\n");
      exit(1);
    }

    printf("What should the timeout be?\n");
    if(scanf("%d", &ll.timeout) != 1)
    {
      printf("Invalid number\n");
      return 1;
    }

    printf("What should the max attemps be?\n");
    if(scanf("%d", &ll.numTransmissions) != 1)
    {
      printf("Invalid number\n");
      return 1;
    }

    printf("What should the frame size be?\n");
    if(scanf("%d", &ll.frameSize) != 1)
    {
      printf("Invalid number\n");
      return 1;
    }

    if(llopen(app.fd, RECEIVER)) return 1;

    if(llread(app.fd, result)) return 1;

    /* 
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no gui�o 
  */

    tcsetattr(app.fd,TCSANOW,&oldtio);
    llclose(app.fd);
    printf("Program executed correctly!\n");
    return 0;
}

