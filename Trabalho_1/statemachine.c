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
#include "statemachine.h"
#include "macros.h"

int isWanted(unsigned char byte, state_machine_st* sm) {
  for (int i = 0; i < sm->wantedBytesLength; i++) {
    if (sm->wantedBytes[i] == byte)
      return i;
  }

  return -1;
}

void change_state(state_machine_st* sm, state_st st) {
    sm->state = st;
}

state_machine_st* create_state_machine(unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte) {
    state_machine_st* sm = malloc(sizeof(state_machine_st));
    change_state(sm, START);
    sm->wantedBytes = wantedBytes;
    sm->wantedBytesLength = wantedBytesLength;
    sm->addressByte = addressByte;
    sm->dataLength = 0;
    return sm;
}

void event_handler(state_machine_st* sm, unsigned char byte, unsigned char* frame, int mode) {

    static int i = 0;

    if(mode ==  SUPERVISION){
      switch(sm->state) {

      case START:
          if (byte == FLAG) {
              change_state(sm, FLAG_RCV);
              frame[0] = byte;
          }
          break;

      case FLAG_RCV:
          if (byte == FLAG)
              break;
          else if (byte == sm->addressByte) {
              change_state(sm, A_RCV);
              frame[1] = byte;
          }
          else
              change_state(sm, START);
          break;

      case A_RCV:
          if (byte == FLAG)
              change_state(sm, FLAG_RCV);
          else if (isWanted(byte, sm)!=-1){
              change_state(sm, C_RCV);
              frame[2] = byte;
          }
          else
              change_state(sm, START);
          break;

      case C_RCV:
          if (byte == createBCC(frame[1], frame[2])){
              change_state(sm, BCC_OK);
              frame[3] = byte;
          }

          else if (byte == FLAG)
              change_state(sm, FLAG_RCV);
          else
              change_state(sm, START);
          break;

      case BCC_OK:
          if (byte == FLAG){
              change_state(sm, STOP);
              frame[4] = byte;
          }
          else
              change_state(sm, START);
          break;

      default:
          break;

    }
  }

    else if(mode == INFORMATION){

    switch(sm->state) {

        case START:
            i = 0;
            if (byte == FLAG) {
                change_state(sm, FLAG_RCV);
                frame[i++] = byte;
            }
            break;

        case FLAG_RCV:
            if (byte == FLAG)
                break;
            else if (byte == sm->addressByte) {
                change_state(sm, A_RCV);
                frame[i++] = byte;
            }
            else {
                change_state(sm, START);
                i = (int) sm->state;
            }
            break;

        case A_RCV:
            if (byte == FLAG) {
                change_state(sm, FLAG_RCV);
                i = (int) sm->state;
            }
            else {
              int n;
              if ((n = isWanted(byte, sm)) >= 0){
                sm->foundIndex = n;
                change_state(sm, C_RCV);
                frame[i++] = byte;
              }
              else {
                change_state(sm, START);
                i = (int) sm->state;
              }
            }
            break;

        case C_RCV:
            if (byte == createBCC(frame[1], frame[2])){
                change_state(sm, BCC_OK);
                frame[i++] = byte;
            }
            else {
              if (byte == FLAG)
                change_state(sm, FLAG_RCV);
              else
                change_state(sm, START);

              i = (int) sm->state;
            }
            break;

        case BCC_OK:
            if(byte ==  FLAG){
              frame[i] = byte;
              change_state(sm, STOP);
              sm->dataLength = i-4;
            }
          else{
              frame[i++] = byte;
          }
          break;


        default:
            break;

    }
  }


}


void destroy_st(state_machine_st* sm) {
  free(sm);
}
