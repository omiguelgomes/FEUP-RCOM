#include "macros.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

void set_state(char byte, states *state);

void ua_state(char byte, states *state);