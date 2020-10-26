
#include "writer.h"

volatile int END=FALSE;

int fd,c, res;
struct termios oldtio,newtio;
extern applicationLayer app;
extern linkLayer ll;

int main(int argc, char** argv)
{

    char buf[255];
    int i, sum = 0, speed = 0;
    
    if (argc != 2) {
      printf("Usage:\n./writer port_number\n");
      exit(1);
    }
    
    app.fd = atoi(argv[1]);

    if(app.fd != 0 || app.fd != 1 || app.fd != 10 || app.fd != 11){
      printf("Port number must be {0, 1, 10, 11}");
      exit(1);
    }

    //default open, no flags
    llopen(app.fd, TRANSMITER);
    printf("Enter a line to be transmitted: \n");

    //recomended function(gets) is dangerous and deprecated
    fgets(buf, 255, stdin);


    //Create new buffer with exact size of string received, no trash
    int counter = 0;
    while(buf[counter] != '\0')
    {
      counter++;
    }

    int bufferSize = counter;
    counter = 0;
    char newBuffer[bufferSize + 1]; //acount for '\0' in the end

    while(counter < bufferSize)
    {
      newBuffer[counter] = buf[counter];
      counter++;
    }
    newBuffer[counter] = '\0';

    //done with the new buffer

    int charWritten = llwrite(fd, newBuffer, counter);
    if (charWritten < 0)
      return 1;

    printf("%d bytes written\n", charWritten);

    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    
    return llclose(fd);
}
