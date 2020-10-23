#include "macros.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

void set_state(char byte, states *state)
{
    switch (*state)
    {
        case START:
            if(byte == FLAG)
                *state = FLAG_RCV;
            break;

        case FLAG_RCV:
            if(byte == A_EMIS)              // usar A_RECE ou A_EMIS ??
                *state = A_RCV;
            else if(byte != FLAG){
                *state = START;
            }
            break;

        case A_RCV:
            if(byte == C_SET)               // usar C_SET ou C_UA ??
                *state = C_RCV;
            else if(byte == FLAG)
                *state = FLAG_RCV;
            else *state = START;
            break;

        case C_RCV:
            if(byte == BCC(A_EMIS, C_SET))  // usar C_SET ou C_UA // usar A_RECE ou A_EMIS ??
                *state = BCC_OK;
            else if(byte == FLAG)
                *state = FLAG_RCV;
            else *state = START;
            break;
        
        case BCC_OK:
            if (byte == FLAG)
                *state = STOP;
            else *state = START;
            break;
    }
}

void ua_state(char byte, states *state)
{
    switch (*state)
    {
        case START:
            if(byte == FLAG)
                *state = FLAG_RCV;
            break;

        case FLAG_RCV:
            if(byte == A_RECE)              // usar A_RECE ou A_EMIS ??
                *state = A_RCV;
            else if(byte != FLAG){
                *state = START;
            }
            break;

        case A_RCV:
            if(byte == C_UA)               // usar C_SET ou C_UA ??
                *state = C_RCV;
            else if(byte == FLAG)
                *state = FLAG_RCV;
            else *state = START;
            break;

        case C_RCV:
            if(byte == BCC(A_RECE, C_UA))  // usar C_SET ou C_UA // usar A_RECE ou A_EMIS ??
                *state = BCC_OK;
            else if(byte == FLAG)
                *state = FLAG_RCV;
            else *state = START;
            break;
        
        case BCC_OK:
            if (byte == FLAG)
                *state = STOP;
            else *state = START;
            break;
    }
}