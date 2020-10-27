/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "writer.h"


#include <strings.h>
#include <string.h>
#include <stdlib.h> //for exit
#include <fcntl.h> // for open
#include <unistd.h> // for close

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

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
      if (buf[res]=='\0') STOP=TRUE;
    }

    printf("Confirmation received!\n");

   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    
    return llclose(fd);
}

//como saber se isto deu erro? write devolve
//sp o nr de chars enviados
int llwrite(int fd, char * buffer, int length)
{
    int res = write(fd, buffer, length);
    return res;
}

int llopen(int porta, int flag)
{
    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */

  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");
}

int llclose(int fd)
{
    return close(fd);
}