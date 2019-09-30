#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "aux.h"

typedef enum state {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} state_st;

typedef enum event {
    EV_FLAG_RCV,
    EV_A_RCV,
    EV_C_RCV,
    EV_CHECK_BCC,
    EV_ERR_BCC
} event_st;

typedef struct state_machine {
    state_st state;
} state_machine_st;

void change_state(state_machine_st* sm, state_st st);
  

state_machine_st* create_state_machine();


void event_handler(state_machine_st* sm, unsigned char byte, unsigned char* frame);


