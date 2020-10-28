#ifndef STATE_MACHINE_H_INCLUDED
#define STATE_MACHINE_H_INCLUDED

#include "macros.h"
#include "dataLink.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

typedef enum
{
    START,
    FLAG_OK,
    A_OK,
    C_OK,
    BCC_OK,
    STOP
}states;

void create_set(char *set);

void create_ua(char *ua);

void create_disc(char *disc);

void set_state(char byte, states *state);

void ua_state(char byte, states *state);

void disc_state(char byte, states *state);

#endif