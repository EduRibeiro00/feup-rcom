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


/**
 * Function to check if a byte is contained in the state machine's wantedBytes field
 * @param byte Byte to be checked
 * @param sm State machine for which to check
 * @return Index of the array where the byte was found; negative if byte is not a member
 */
int isWanted(unsigned char byte, state_machine_st* sm);


/**
 * Function to update the state of the state machine
 * @param sm State machine to be updated
 * @param st State to be assigned to the state machine
 */
void change_state(state_machine_st* sm, state_st st);


/**
 * Function to create a state machine, with the given attributes
 * @param wantedBytes Possible bytes that are expected in the frame to be read by the state machine
 * @param wantedBytesLength Number of possible bytes that are expected in the frame to be read by the state machine
 * @param addressByte Address from which the frame to be read by the state machine is expected
 * @return Pointer to the new state machine "object" (struct)
 */
state_machine_st* create_state_machine(unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte);


/**
 * Function to update the state machine according to the bytes read
 * @param sm State machine to be updated
 * @param byte Last byte to have been read, of the current frame
 * @param frame Address where the frame that's being read is being stored
 * @param mode Type of frame that's being read (Supervision or Information)
 */
void event_handler(state_machine_st* sm, unsigned char byte, unsigned char* frame, int mode);


/**
 * Function to free the memory allocated to a state machine object
 * @param sm State machine to be destroyed
 */
void destroy_st(state_machine_st* sm);
