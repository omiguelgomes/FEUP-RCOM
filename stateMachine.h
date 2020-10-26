#ifndef STATE_MACHINE_H_INCLUDED
#define STATE_MACHINE_H_INCLUDED

#include "macros.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

typedef enum
{
    START = 0,
    FLAG_OK = 1,
    A_OK = 2,
    C_OK = 3,
    BCC_OK = 4,
    STOP = 5
}states;

void set_state(char byte, enum states *state);

void ua_state(char byte, enum states *state);

#endif