#include "dataLink.h"
#include "stateMachine.c"

int c, res;
struct termios oldtio,newtio;

applicationLayer app;
linkLayer ll;

int llwrite(int fd, char * buffer, int length)
{
    int totalBytesRemaining = length;
    int bytesSent = 0;
    while (totalBytesRemaining > 0)
    {
        int bytesToSend = totalBytesRemaining >= DATA_SIZE ? DATA_SIZE : totalBytesRemaining;
        unsigned char *trama = malloc((bytesToSend + 6) * sizeof(unsigned char));

        trama[0] = FLAG;
        trama[1] = A_SND;
        trama[2] = C_SND;
        trama[3] = BCC(A_SND, C_SND);
        for (int i = 1; i <= bytesToSend; i++)
        {
            trama[3+i] = buffer[bytesSent];
            bytesSent++;
        }
        trama[4 + bytesToSend] = BCC(A_SND, C_SND);
        trama[5 + bytesToSend] = FLAG;

        // for (int i = 4; i < 4 + bytesToSend; i++)
        // {
        //     printf("trama[%d]: %c\n", i, trama[i]);
        // }

        // for (int i = 0; i < 9; i++)
        // {
        //     printf("Sent this byte %d: %#x\n", i, trama[i]);
        // }

        int res = write(fd, trama, bytesToSend + 6);
        totalBytesRemaining -= bytesToSend;
    }
    return res;
}


int llread(int fd, char * buffer)
{
    read(fd, buffer, MAX_SIZE);
    // if (buffer[0] != FLAG) return 1;
    // if(buffer[1] != A_SND) return 1;
    // if(buffer[2] != C_SND) return 1;
    // if(buffer[3] != BCC(A_SND, C_SND)) return 1;

    //current string sent comes in with 3 bytes of info at a time, will have to be changed
    for (int i = 4; i < 7; i++)
    {
        printf("buffer[%d]: %c\n", i, buffer[i]);
    }
    
    // if(buffer[i+1] != FLAG)
    //     return 1;
    return 0;
}

int set_termios()
{
    printf("setting up termios...\n");

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
    tcflush(app.fd, TCIOFLUSH);

    printf("Starting llopen!\n");

    if (status != TRANSMITER && status != RECEIVER) return -1;

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
    }

    app.fd = open(ll.port, O_RDWR | O_NOCTTY);
    if(app.fd < 0)
    {
        perror(ll.port);
        exit(-1);
    }

    set_termios();

    if(app.status == TRANSMITER)
    {

        // TRANSMITER

        // Send SET

        char set[5];
        create_set(set);

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
        // READER

        // Receive SET

        states state = START;
        int result[MAX_SIZE];

        for(int i = 0; state != STOP; i++)
        {
            res = read(app.fd, &result[i], 1);
            printf("Receiving SET byte %d: %#x\n", i, result[i]);
            set_state(result[i], &state);
        }

        // Send UA
        
        char *ua = malloc(6*sizeof(char));

        create_ua(ua);
        printf("Sending UA ...\n");
        res = write(app.fd, ua, 5);
    }

    printf("New termios structure set\n");

    return app.fd;
}


int llclose(int fd)
{
    int res;
    char disc_rcv[5], disc_snd[5], ua[5];
    states state;

    //CHECK IF THEY ARE ACTUALL ALTERED
    printf("disc before: %s\n", disc_snd);
    printf("ua before: %s\n", ua);
    create_disc(disc_snd);
    create_ua(ua);
    printf("disc after: %s\n", disc_snd);
    printf("ua after: %s\n", ua);

    if(app.status == TRANSMITER) //case transmiter
    {   
        // send disc
        printf("Sending DISC ...\n");
        res = write(app.fd, disc_snd, 5);
           
        // wait for disc
        state = START;
        for(int i = 0; state != STOP; ++i)
        {
            read(fd, &disc_rcv, 1);
            printf("Receiving DISC byte %d: %#x\n", i, disc_rcv[i]);
            disc_state(disc_rcv[i], &state);
        }

        // send ua

        printf("Sending UA ...\n");
        res = write(app.fd, ua, 5);
    }
    else //case receiver
    {   
        // wait for disc
        state = START;
        for(int i = 0; state != STOP; ++i)
        {
            read(fd, &disc_rcv, 1);
            printf("Receiving DISC byte %d: %#x\n", i, disc_rcv[i]);
            disc_state(disc_rcv[i], &state);
        }
        
        // send disc
        printf("Sending DISC ...\n");
        res = write(app.fd, disc_snd, 5);

        // wait for ua
        state = START;
        for(int i = 0; state != STOP; ++i)
        {
            read(fd, &ua, 1);
            printf("Receiving UA byte %d: %#x\n", i, ua[i]);
            ua_state(ua[i], &state);
        }
    }

    return close(fd);
}























