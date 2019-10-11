#pragma once

typedef enum state {
    START = 0,
    FLAG_RCV,
    A_RCV,
    C_RCV,
    BCC_OK,
    STOP
} state_st;

typedef struct state_machine {
    state_st state;
    unsigned char* wantedBytes;
    int wantedBytesLength;
    unsigned char addressByte;
    int foundIndex;
    int dataLength;
} state_machine_st;


int isWanted(unsigned char byte, state_machine_st* sm);


void change_state(state_machine_st* sm, state_st st);


state_machine_st* create_state_machine(unsigned char* wantedByte, int wantedBytesLength, unsigned char addressByte);


void event_handler(state_machine_st* sm, unsigned char byte, unsigned char* frame, int mode);


void destroy_st(state_machine_st* sm);
