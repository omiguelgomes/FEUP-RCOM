
#include "writer.h"

volatile int STOP=FALSE;

int fd,c, res;
struct termios oldtio,newtio;

int main(int argc, char** argv)
{

    char buf[255];
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS10\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    //default open, no flags
    llopen(fd, 0);
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
