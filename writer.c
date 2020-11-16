#include "writer.h"

volatile int END=FALSE;

int fd,c, res;
struct termios oldtio,newtio;
applicationLayer app;
linkLayer ll;

int main(int argc, char** argv)
{
    // Calculate the time taken by the transfer
    clock_t t;
    t = clock();

    char buf[30000];
    int i, sum = 0, speed = 0;

//    if (argc != 3) {
//      printf("Usage:\n./writer port_number file\n");
//      exit(1);
//    }


    //app.fd = atoi(argv[1]);
    app.fd =11;

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

//    printf("What should the baudrate be?\n");
//    if(scanf("%d", BAUDRATE) != 1)
//    {
//        printf("Invalid number\n");
//        return 1;
//    }
//    char cwd[255];
//    getcwd(cwd, sizeof(cwd));
//    printf(cwd);
    //strcpy(ll.fileName, "pinguim2.gif");
    strcpy(ll.fileName, "ola2.txt");
    //strcpy(ll.fileName, "test.png");
    //strcpy(ll.fileName, argv[2]);
    int fileSize = openFile(buf, "/home/omiguelgomes/Desktop/FEUP/20-21/RCOM/Project/cmake-build-debug/ola2.txt");

    if(llopen(app.fd, TRANSMITER)) return 1;

    if(llwrite(app.fd, buf, fileSize)) return 1;
   
    if ( tcsetattr(app.fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
    if(llclose(app.fd) == 1)
    {
      printf("llclose failed\n");
      return 1;
    } 

    t = clock() - t;
    double time_taken = ((double)t)/CLOCKS_PER_SEC; // calculate the elapsed time
    printf("The program took %f seconds to execute\n", time_taken);
    printf("The transfer rate was %f kBytes/second\n", (fileSize/8)/(time_taken*1000));
    return 0;
}
