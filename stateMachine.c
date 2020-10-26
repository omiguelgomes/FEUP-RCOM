#include "stateMachine.h"

void create_set(char *set)
{
    set[0] = FLAG;
    set[1] = A_RCV;
    set[2] = C_RCV;
    set[3] = BCC(A_RCV, C_RCV);
    set[4] = FLAG;
}

void create_ua(char *ua)
{
    ua[0] = FLAG;
    ua[1] = A_SND;
    ua[2] = C_SND;
    ua[3] = BCC(A_SND, C_SND);
    ua[4] = FLAG;
}

void set_state(char byte, states *state)
{
    switch (*state)
    {
        case START:
            if(byte == FLAG)
                *state = FLAG_OK;
            break;

        case FLAG_OK:
            if(byte == A_RCV)   
                *state = A_OK;
            else if(byte != FLAG){
                *state = START;
            }
            break;

        case A_OK:
            if(byte == C_RCV)
                *state = C_OK;
            else if(byte == FLAG)
                *state = FLAG_OK;
            else *state = START;
            break;

        case C_OK:
            if(byte == BCC(A_RCV, C_RCV))
                *state = BCC_OK;
            else if(byte == FLAG)
                *state = FLAG_OK;
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
                *state = FLAG_OK;
            break;

        case FLAG_OK:
            if(byte == A_SND)
                *state = A_OK;
            else if(byte != FLAG){
                *state = START;
            }
            break;

        case A_OK:
            if(byte == C_SND)
                *state = C_OK;
            else if(byte == FLAG)
                *state = FLAG_OK;
            else *state = START;
            break;

        case C_OK:
            if(byte == BCC(A_SND, C_SND))
                *state = BCC_OK;
            else if(byte == FLAG)
                *state = FLAG_OK;
            else *state = START;
            break;
        
        case BCC_OK:
            if (byte == FLAG)
                *state = STOP;
            else *state = START;
            break;
    }
}
