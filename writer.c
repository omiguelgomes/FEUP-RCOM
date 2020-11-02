#include "writer.h"

volatile int END=FALSE;

int fd,c, res;
struct termios oldtio,newtio;
applicationLayer app;
linkLayer ll;

int main(int argc, char** argv)
{

    char buf[255];
    int i, sum = 0, speed = 0;

    if (argc != 3) {
      printf("Usage:\n./writer port_number\n");
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

    int fileSize = openFile(*buf, argv[2]);

    //printf("%s\n", buf);

    llopen(app.fd, TRANSMITER);

    llwrite(app.fd, "ola adeus", 10);
    //llwrite(app.fd, buf, fileSize);

   
    if ( tcsetattr(app.fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    llclose(app.fd);

    printf("Program executed correctly!\n");
    return 0;
    //return llclose(fd);
}
