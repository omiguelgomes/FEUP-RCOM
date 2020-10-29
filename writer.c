#include "writer.h"
#include "macros.h"
#include "dataLink.h"

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

    /*for (i = 0; i < 255; i++) {
      buf[i] = 'a';
    }*/

    fgets(buf, sizeof(buf), stdin);

    int len = strlen(buf);
    
    /*testing*/
    //buf[25] = '\n';
    
    res = write(fd,buf,len);
    printf("%d bytes written\n", res);
 
    printf("waiting for confirmation...\n");

  /* 
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar 
    o indicado no gui�o 
  */

    while (STOP==FALSE) {
      res = read(fd,buf,255);
      buf[res]='\0';
      printf("%s", buf);
      if (buf[res]=='\0'){
        //STOP=TRUE;
        printf("stop =true");
      }
    }

    printf("Confirmation received!\n");

   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    
    return llclose(fd);
}
