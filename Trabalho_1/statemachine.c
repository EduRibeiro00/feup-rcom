#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

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

void change_state(state_machine_st sm, state_st st) {
    sm.state = st;
}

state_machine_st create_state_machine() {
    state_machine_st sm;
    change_state(sm, START);
    return sm;
}

void event_handler(state_machine_st sm, event_st ev) {

    switch(sm.state) {

        case START:
            if (ev == EV_FLAG_RCV)
                change_state(sm, FLAG_RCV);
            break;

        case FLAG_RCV:
            if (ev == EV_FLAG_RCV)
                break;
            else if (ev == EV_A_RCV)
                change_state(sm, A_RCV);
            break;

        case A_RCV:
            if (ev == EV_FLAG_RCV)
                change_state(sm, FLAG_RCV);
            else if (ev == EV_C_RCV)
                change_state(sm, C_RCV);
            else
                change_state(sm, START);
            break;

        case C_RCV:
            if (ev == EV_CHECK_BCC)
                change_state(sm, BCC_OK);
            else if (ev == EV_FLAG_RCV)
                change_state(sm, FLAG_RCV);
            else
                change_state(sm, START);            
            break;

        case BCC_OK:
            if (ev == EV_FLAG_RCV)
                change_state(sm, STOP);
            else
                change_state(sm, START);            
            break;

        default:
            break;

    };

}
