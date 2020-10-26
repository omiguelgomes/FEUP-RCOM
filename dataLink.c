#include "dataLink.h"

volatile int STOP=FALSE;

int c, res;
struct termios oldtio,newtio;

extern applicationLayer app;
extern linkLayer ll;

//como saber se isto deu erro? write devolve
//sp o nr de chars enviados
int llwrite(int fd, char * buffer, int length)
{
    int res = write(fd, buffer, length);
    return res;
}


int llread(int fd, char * buffer)
{
    return 1;
}

int set_termios()
{
    if ( tcgetattr(app.fd, &oldtio) == -1) { /* save current port settings */
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
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 5 chars received */

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
    */

    tcflush(app.fd, TCIOFLUSH);

    if ( tcsetattr(app.fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
}


int llopen(int port, int status)
{

    printf("Starting llopen!\n");

    if (status != TRANSMITER || status != RECEIVER) return -1;

    app.status = status;

    ll.sequenceNumber = 0;

    switch (port)
    {
    case HOME_PORT_0:
        strcpy(ll.port, "/dev/ttyS10");
        break;
    case HOME_PORT_1:
        strcpy(ll.port, "/dev/ttyS11");
        break;
    case LAB_PORT_0:
        strcpy(ll.port, "/dev/ttyS0");
        break;
    case LAB_PORT_1:
        strcpy(ll.port, "/dev/ttyS1");
        break;
    default:
        return -1;
        break;
    }

    app.fd = open(ll.port, O_RDWR | O_NOCTTY);
    if(app.fd < 0)
    {
        perror(ll.port);
        exit(-1);
    }

    set_termios();

    if(app.status == TRANSMITER){

        // TRANSMITER

        // Send SET

        char set[5];
        create_set(*set);
        //memcpy(ll.frame, set, 5);
        printf("Sending SET ...\n");
        res = write(app.fd, set, 5);

        // Receive UA

        states state = START;
        int result[MAX_SIZE];

        for(int i = 0; state != STOP; ++i)
        {
            res = read(app.fd, &result[i], 1);
            printf("Receiving UA byte %d: %#x\n", i, result[i]);
            ua_state(result[i], &state);
        }
    }
    else
    {

        // RECEIVER

        // Receive SET

        states state = START;
        int result[MAX_SIZE];

        for(int i = 0; state != STOP; ++i)
        {
            res = read(app.fd, &result[i], 1);
            printf("Receiving SET byte %d: %#x\n", i, result[i]);
            set_state(result[i], &state);
        }

        // Send UA
        
        char ua[5];

        create_ua(*ua);
        printf("Sending UA ...\n");
        res = write(app.fd, ua, 5);
    }

    printf("New termios structure set\n");

    return app.fd;
}


int llclose(int fd)
{
    return close(fd);
}