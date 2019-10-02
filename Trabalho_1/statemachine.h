#pragma once

typedef enum state {
    START,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} state_st;



typedef struct state_machine {
    state_st state;
} state_machine_st;

void change_state(state_machine_st* sm, state_st st);


state_machine_st* create_state_machine();


void event_handler(state_machine_st* sm, unsigned char byte, unsigned char* frame);


void destroy_st(state_machine_st* sm);
