#include "dataLink.h"

struct termios oldtio,newtio;

applicationLayer app;
linkLayer ll;
int count = 0;

void alarmHandler()
{
    tcflush(app.fd, TCIOFLUSH);

    if ((ll.numTransmissions - 1) == count)
    {
        count++;
        printf("Can't connect to receiver! (failed attempt number %d) \n", count);
        if (tcsetattr(app.fd, TCSANOW, &oldtio) == -1)
        {
            perror("tcsetattr");
            close(app.fd);
            exit(-1);
        }
        close(app.fd);
        exit(-1);
    }
    count++;
    printf("Connection timed out. Retrying...(failed attempt number %d) \n", count);
    if (app.status == TRANSMITER)
        write(app.fd, ll.frame, ll.frameSize);
    alarm(ll.timeout);
}

int stuffing(const unsigned char *info, size_t size, unsigned char *stuffed_info)
{

    int i, j;
    for (i = 0, j = 0; i < size; i++, j++)
    {
        switch (info[i])
        {
        case FLAG:
            stuffed_info[j] = ESC;
            stuffed_info[j + 1] = MASKED_FLAG;
            j++;
            break;
        case ESC:
            stuffed_info[j] = ESC;
            stuffed_info[j + 1] = MASKED_ESC;
            j++;
            break;
        default:
            stuffed_info[j] = info[i];
            break;
        }
    }

    if (info == NULL || stuffed_info == NULL)
        return -1;

    return j;
}

int destuffing(const unsigned char *stuffed_info, size_t size, unsigned char *info)
{
    int i, j;
    for (i = 0, j = 0; j < size; i++, j++)
    {
        if (stuffed_info[j] == ESC)
            switch (stuffed_info[j + 1])
            {
            case MASKED_ESC:
                info[i] = ESC;
                j++;
                break;
            case MASKED_FLAG:
                info[i] = FLAG;
                j++;
                break;
            default:
                break;
            }
        else
            info[i] = stuffed_info[j];
    }
    if (stuffed_info == NULL || info == NULL)
        return -1;

    return i;
}

int openFile(char *buffer, char *fileName)
{
    FILE *fp;
    long lSize;

    fp = fopen (fileName , "rb" );
    if( !fp ) perror(fileName),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
      fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

    /* do your work here, buffer is a string contains the whole text */

    fclose(fp);
    free(buffer);

    return lSize;
}

int llwrite(int fd, char *buffer, int length)
{
    printf("Starting llwrite!\n");
    char result[255];
    int res;
    int rej = 0;
    create_frame(buffer, length);
    do
    {
        printf("Writing frame!\n");
        write(fd, ll.frame, ll.frameSize);
        alarm(ll.timeout);

        states state = START;
        for (int i = 0; state != STOP; i++)
        {
            if (res = read(fd, &result[i], 1) == -1)
                return -1;
            printf("Receiving CONFIRMATION byte %d: %#x\n", i, result[i]);
            supervision_state(result[i], &state);
        }
        if (result[3] == C_I(ll.sequenceNumber))
        {
            ll.sequenceNumber ^= 1;
            rej = 0;
        }
        else if (result[3] == C_REJ(ll.sequenceNumber))
        {
            count++;
            rej = 1;
        }
        alarm(0);
    } while (rej);
    return res;
}

int llread(int fd, char *buffer)
{
    printf("Starting llread!\n");
    int res;
    int stuffed_size = 0;
    char result[255];
    char message[255];
    states state = START;
    for (int i = 0; state != STOP; i++)
    {
        read(fd, &buffer[i], 1);
        printf("Receiving FRAME byte %d: %#x\n", i, buffer[i]);
        info_state(buffer[i], &state);
        stuffed_size++;
    }
    int info_size = destuffing(buffer, stuffed_size, result);

    unsigned char bcc = result[4];
    for (int i = 5; i < info_size - 2; i++)
    {
        bcc ^= result[i];
    }

    if (bcc == result[info_size - 2])
    {
        printf("Creating CONFIRMATION\n");
        create_rr(message);
        ll.sequenceNumber ^= 1;
    }
    else
    {
        printf("Creating REJ\n");
        create_rej(message);
    }

    write(fd, message, ll.frameSize);
    return stuffed_size;
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
    printf("Starting llopen!\n");

    if (status != TRANSMITER && status != RECEIVER) return -1;

    app.status = status;

    ll.sequenceNumber = 0;
    ll.frameSize = 5;

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

        int res;
        char set[5];
        signal(SIGALRM, alarmHandler);
        create_set(set);
        memcpy(ll.frame, set, ll.frameSize);

        printf("Sending SET ...\n");
        res = write(app.fd, set, ll.frameSize);

        alarm(ll.timeout);

        // Receive UA

        states state = START;
        char result[MAX_SIZE];

        for(int i = 0; state != STOP; ++i)
        {
            res = read(app.fd, &result[i], 1);
            printf("Receiving UA byte %d: %#x\n", i, result[i]);
            ua_state(result[i], &state);
        }
        alarm(0);
    }
    else
    {
        // READER

        // Receive SET

        int res;
        states state = START;
        char result[MAX_SIZE];

        for(int i = 0; state != STOP; ++i)
        {
            res = read(app.fd, &result[i], 1);
            printf("Receiving SET byte %d: %#x\n", i, result[i]);
            set_state(result[i], &state);
        }

        // Send UA

        char ua[5];

        create_ua(ua);
        printf("Sending UA ...\n");
        res = write(app.fd, ua, ll.frameSize);

    }

    printf("llopen executed correctly\n");

    return app.fd;
}

int llclose(int fd)
{

    printf("\nStarting llclose...\n");
    int res;
    char disc_rcv[5], disc_snd[5], ua[5];
    states state;
    

    printf("Creating DISCONNECT!\n");
    create_disc(disc_snd);
    printf("Creating UA!\n");
    create_ua(ua);

    if(app.status == TRANSMITER) //case transmiter
    {   
        // send disc

        printf("Sending DISC ...\n");
        res = write(app.fd, disc_snd, ll.frameSize);
           
        // wait for disc
        alarm(ll.timeout);
        state = START;
        for(int i = 0; state != STOP; ++i)
        {
            read(fd, &disc_rcv[i], 1);
            printf("Receiving DISC byte %d: %#x\n", i, disc_rcv[i]);
            disc_state(disc_rcv[i], &state);
        }

        // send ua

        printf("Sending UA ...\n");
        res = write(app.fd, ua, ll.frameSize);
    }
    else //case receiver
    {   
        // wait for disc
        state = START;
        //for(int i = 0; state != STOP; ++i)
        for(int i = 0; state != STOP; ++i)
        {
            read(fd, &disc_rcv[i], 1);
            printf("Receiving DISC byte %d: %#x\n", i, disc_rcv[i]);
            disc_state(disc_rcv[i], &state);
        }
        
        // send disc
        alarm(ll.timeout);
        printf("Sending DISC ...\n");
        res = write(app.fd, disc_snd, ll.frameSize);

        // wait for ua
        state = START;
        for(int i = 0; state != STOP; ++i)
        {
            read(fd, &ua[i], 1);
            printf("Receiving UA byte %d: %#x\n", i, ua[i]);
            ua_state(ua[i], &state);
        }
        alarm(0);
    }

    return close(fd);
}
























