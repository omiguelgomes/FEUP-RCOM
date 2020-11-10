#include "dataLink.h"
#include <stdbool.h>
#include<errno.h>

struct termios oldtio,newtio;

applicationLayer app;
linkLayer ll;
int count = 0;
bool alarmFlag = FALSE;

void alarmHandler()
{
    printf("Got an alarm\n");
    count++;
    alarmFlag = TRUE;
    alarm(ll.timeout);
    siginterrupt(SIGALRM, 1);
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
    char *c;

    fp = fopen (fileName , "r" );
    if( !fp ) perror(fileName),exit(1);

    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );

    
    for (int i = 0; i < lSize; i++)
    {
        buffer[i] = fgetc(fp);
    }

    fclose(fp);

    return lSize;
}

int llwrite(int fd, char *buffer, int length)
{
    printf("\nStarting llwrite!\n");
    char result[255];
    int res;
    int rej = 0;
    char splitBuffer[ll.frameSize];
    int frameDataSize = ll.frameSize;

    //rounds totalFrames up
    int totalFrames = length % ll.frameSize == 0 ? length / ll.frameSize : length / ll.frameSize + 1;
    
    int bytesNeeded = sizeof(length);
    //CONTROL PACKET BEFORE TRANSMISSION
    unsigned char controlPacket[5+strlen(ll.fileName)+bytesNeeded];
    //indicate start
    controlPacket[0] = 2;
    //indicate its filesize
    controlPacket[1] = 0;

    //nr of bytes to show fileSize
    controlPacket[2] = bytesNeeded;
    //actual fileSize
    for (int i = 0; i < bytesNeeded; i++)
    {
        controlPacket[3 + i] = (length >> (8 * i))& 0xff;
    }
    
    //indicate its its fileName
    controlPacket[3+bytesNeeded] = 1;
    //nr of bytes to show fileName
    controlPacket[4+bytesNeeded] = strlen(ll.fileName);
    //actual fileName
    for (int i = 0; i < strlen(ll.fileName); i++)
    {
        controlPacket[5+bytesNeeded+i] = ll.fileName[i];
    }

    write(app.fd, controlPacket, 5 + strlen(ll.fileName)+bytesNeeded);

    //for each frame needed
    for (int j = 0; j < totalFrames; j++)
    {
        strncpy(splitBuffer, buffer + j*frameDataSize, frameDataSize);
        create_frame(splitBuffer, frameDataSize);
        
        write(fd, ll.frame, ll.frameSize);

        // states state = START;
        // alarm(ll.timeout);
        // for (int i = 0; state != STOP; i++)
        // {
        //     read(fd, &result[i], 1);
        //     supervision_state(result[i], &state);
        //     //printf("result %d: %d\n", i, result[i]);
        //     if(state == 3)
        //     {
        //         if (result[i] == C_I(ll.sequenceNumber))
        //         {
        //             printf("Gonna send the next one\n");
        //             ll.sequenceNumber++;
        //             rej = 0;
        //         }
        //         else if (result[i] == C_REJ(ll.sequenceNumber))
        //         {
        //             printf("Gonna resend this one\n");
        //             j--;
        //         }
        //     }
        // }
        alarm(0);
    }

    //RESEND CONTROL PACKET
    controlPacket[0] = 3;
    write(app.fd, controlPacket, 5 + strlen(ll.fileName+bytesNeeded));
    return 0;
}

int llread(int fd, char *buffer)
{
    //RECEIVE CONTROL BYTE
    ctrl_states ctrlStates = START_CTRL;
    unsigned char bufferCntrl;
    unsigned int fileSize = 0;
    int bytesForFileName;
    int bytesForSize;

    while(ctrlStates != STOP_CTRL)
    {
        read(fd, &bufferCntrl, 1);
        switch(ctrlStates)
        {
            case(START_CTRL):
                if(bufferCntrl == 2)
                    ctrlStates = T;
                break;
            case(T):
                if(bufferCntrl == 0)
                {
                    ctrlStates = L;
                }
                break;
            case(L):
                bytesForSize = bufferCntrl;
                for (int i = 0; i < bytesForSize; i++)
                {
                    read(fd, &bufferCntrl, 1);
                    fileSize += (bufferCntrl << (8 * i));
                }
                    ctrlStates = T2;
                break;
            case(T2):
                if(bufferCntrl == 1)
                    ctrlStates = L2;
                break;
            case(L2):
                bytesForFileName = bufferCntrl;
                for (int i = 0; i < bytesForFileName; i++)
                {
                    read(fd, &bufferCntrl, 1);
                    ll.fileName[i] = bufferCntrl;
                }
                ctrlStates = STOP_CTRL;
                break;
        }
    }
    //printf("Received control byte\n");
    int totalFrames = fileSize % ll.frameSize == 0 ? fileSize / ll.frameSize : fileSize / ll.frameSize + 1;
    char *finalFile = (char *)malloc(sizeof(char) * fileSize);
    ll.fileSize = fileSize;
    int bytesWritten = 0;
    for (int j = 0; j < totalFrames; j++)
    {
        printf("\nStarting llread!\n");
        int res;
        char result[ll.frameSize];
        int stuffed_size = 0;
        char message[5];
        states state = START;
        unsigned char bcc;
        for (int i = 0; state != STOP; i++)
        {  
            read(fd, &buffer[i], 1);
            info_state(buffer[i], &state);
            if(state == BCC_OK)
            {
                bcc = buffer[i];
            }
            stuffed_size++;   
        }

        int info_size = destuffing(buffer, stuffed_size, result);

        for (int i = 5; i < info_size - 2; i++)
        {
            bcc ^= result[i];
        }

        memcpy(&finalFile[strlen(finalFile)], result + 4, ll.frameSize);
        bytesWritten += info_size;

        // if (bcc == result[info_size - 2])
        // {
        //     printf("Creating CONFIRMATION\n");
        //     create_rr(message);
        //     ll.sequenceNumber++;
        // }
        // else
        // {
        //     printf("Creating REJ\n");
        //     create_rej(message);
        // }
        // write(fd, message, 5);
    }
    memcpy(buffer, finalFile, strlen(finalFile));
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

    //default timer, should be 0 since we implemented our own
    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    //minimum of chars read, must be at least 1
    newtio.c_cc[VMIN]     = 1;   

    /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
    */

    //tcflush(app.fd, TCIOFLUSH);

    if (tcsetattr(app.fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
}


int llopen(int port, int status)
{
    printf("\nStarting llopen!\n");

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
    signal(SIGALRM, alarmHandler);

    if(app.status == TRANSMITER)
    {

        // TRANSMITER

        // Send SET
        int res;
        char set[5];
        create_set(set);

        printf("Sending SET ...\n");
        res = write(app.fd, set, ll.frameSize);

        // Receive UA
        states state = START;
        char result;
        alarm(ll.timeout);
        for (int i = 0; state != STOP; ++i)
        {
            read(app.fd, &result, 1);
            ua_state(result, &state);

            if(alarmFlag && count > ll.numTransmissions)
            {
                printf("Max attempts reached, disconnecting\n");
                return 1;
            }
            if(alarmFlag)
            {
                printf("Didn't receive UA, resending SET\n");
                res = write(app.fd, set, 5);
                alarmFlag = FALSE;
            }
        }
        alarm(0);

    }
    else
    {
        // READER

        // Receive SET
        int res;
        states state = START;
        char result;
        alarm(ll.timeout);
        for (int i = 0; state != STOP; ++i)
        {
            res = read(app.fd, &result, 1);
            set_state(result, &state);
            if (alarmFlag && count > ll.numTransmissions)
            {
                printf("Max attempts reached, disconnecting\n");
                return 1;
            }
            if(alarmFlag)
            {
                alarmFlag = FALSE;
            }
        }
        printf("Received SET\n");
        alarm(0);

        // Send UA
        char ua[5];
        create_ua(ua);
        printf("Sending UA ...\n");
        res = write(app.fd, ua, ll.frameSize);
    }

    printf("llopen executed correctly\n");

    return 0;
}


int llclose(int fd)
{
    printf("\nStarting llclose...\n");
    int res;
    
    states state;

    if (app.status != TRANSMITER && app.status != RECEIVER) return -1;

    if(app.status == TRANSMITER) //case transmiter
    {   
        // SEND DISC
        printf("Creating DISCONNECT!\n");
        char disc_snd[5];
        create_disc(disc_snd);
        printf("Sending DISC ...\n");
        res = write(app.fd, disc_snd, 5);

        //RECEIVE DISC
        char disc_rcv;
        state = START;
        alarm(ll.timeout);
        for (int i = 0; state != STOP; i++)
        {
            if(alarmFlag && count > ll.numTransmissions)
            {
                printf("Max attempts reached, disconnecting\n");
                return 1;
            }
            if(alarmFlag)
            {
                printf("Didn't receive DISC, resending DISC\n");
                res = write(app.fd, disc_snd, 5);
                alarmFlag = FALSE;
            }
 
            read(app.fd, &disc_rcv, 1);
            disc_state(disc_rcv, &state);
        }
        alarm(0);
        printf("received the whole byte!\n");

        // SEND UA
        char ua[5];
        create_ua(ua);
        printf("Sent UA\n");
        res = write(app.fd, ua, 5);
    }
    else //case receiver
    {   
        // sSEND DISC
        printf("Creating DISCONNECT!\n");
        char disc_snd[5];
        create_disc(disc_snd);
        printf("Sending DISC ...\n");
        res = write(app.fd, disc_snd, 5);

        // RECEIVE UA
        alarm(ll.timeout);
        state = START;
        char ua;
        printf("Gonna receive ua\n");
        state = START;
        for (int i = 0; state != STOP; i++)
        {
            if(alarmFlag && count > ll.numTransmissions)
            {
                printf("Max attempts reached, disconnecting\n");
                return 1;
            }
            if(alarmFlag)
            {
                //printf("Didn't receive UA, resending DISC\n");
                res = write(app.fd, disc_snd, 5);
                alarmFlag = FALSE;
            }

            read(fd, &ua, 1);
            ua_state(ua, &state);
        }
        alarm(0);
        printf("Received ua\n");
        return close(fd);
    }

    return 0;
}