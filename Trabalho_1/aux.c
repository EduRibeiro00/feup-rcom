#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include "app.h"
#include "macros.h"
#include "statemachine.h"
#include "data_link.h"
#include "alarm.h"


unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
}

unsigned char createBCC_2(unsigned char* frame, int length) {

  unsigned char bcc2 = frame[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ frame[i];
  }

  return bcc2;
}


int byte_stuffing(unsigned char* frame, int length) {

  // number of packet bytes in the frame at the beginning, that is going to be on the final frame
  // after byte stuffing
  int counter = 0;

  // allocates space for auxiliary buffer
  unsigned char *aux = malloc(sizeof(unsigned char) * (length + 6));
  if(aux == NULL){
    return -1;
  }

  // allocates maximum space that could be necessary for the frame
  frame = realloc(frame ,sizeof(unsigned char ) * ((length + 1) * 2) + 5);
  if (frame == NULL){
    free(aux);
    return -1;
  }


  // passes information from the frame to aux
  for(int i = 0; i < length + 6 ; i++){
    aux[i] = frame[i];
  }



  int j = DATA_START;


  // parses aux buffer, and fills in correctly the frame buffer
  for(int i = DATA_START; i < (length + 6); i++){

    if(aux[i] == FLAG && i != (length + 5)) {
      frame[j] = ESCAPE_BYTE;
      frame[j+1] = BYTE_STUFFING_FLAG;
      j = j + 2;
      counter++;
    }
    else if(aux[i] == ESCAPE_BYTE && i != (length + 5)) {
      frame[j] = ESCAPE_BYTE;
      frame[j+1] = BYTE_STUFFING_ESCAPE;
      j = j + 2;
      counter++;
    }
    else{
      frame[j] = aux[i];
      j++;
    }
  }

  // reallocates space for the frame buffer in order to occupy only the necessary space
  frame = realloc(frame, sizeof(unsigned char) * (length + 6 + counter));
  if(frame == NULL){
    free(aux);
    return -1;
  }


  free(aux);

  return j;
}


int byte_destuffing(unsigned char* frame, int length) {

  // allocates space for the maximum possible frame length read
  char *aux = malloc(sizeof(unsigned char) * (length + 5));
  if(aux == NULL){
    return -1;
  }

  // copies the content of the frame (with stuffing) to the aux frame
  for(int i = 0; i < (length + 5) ; i++) {
    aux[i] = frame[i];
  }

  int j = DATA_START;

  // iterates through the aux buffer, and fills the frame buffer with destuffed content
  for(int i = DATA_START; i < (length + 5); i++) {

    if(aux[i] == ESCAPE_BYTE){
      if (aux[i+1] == BYTE_STUFFING_ESCAPE) {
        frame[j] = ESCAPE_BYTE;
      }
      else if(aux[i+1] == BYTE_STUFFING_FLAG) {
        frame[j] = FLAG;
      }
      i++;
      j++;
    }
    else{
      frame[j] = aux[i];
      j++;
    }
  }

  // reallocates only the space required for the final frame contents
  frame = realloc(frame, sizeof(unsigned char) * j);
  if(frame == NULL){
    free(aux);
    return -1;
  }


  free(aux);

  return j;
}


int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role) {

    frame[0] = FLAG;

    if(role == TRANSMITTER) {
        if(controlField == SET || controlField == DISC) {
            frame[1] = END_SEND;
        }
        else if(controlField == UA || controlField == RR_0 || controlField == REJ_0 || controlField == RR_1 || controlField == REJ_1 ) {
            frame[1] = END_REC;
        }
        else return -1;
    }
    else if(role == RECEIVER) {
        if(controlField == SET || controlField == DISC) {
            frame[1] = END_REC;
        }
        else if(controlField == UA || controlField == RR_0 || controlField == REJ_0 || controlField == RR_1 || controlField == REJ_1 ) {
            frame[1] = END_SEND;
        }
        else return -1;
    }
    else return -1;

    frame[2] = controlField;

    frame[3] = createBCC(frame[1], frame[2]);

    frame[4] = FLAG;

    return 0;
}


int createInformationFrame(unsigned char* frame, unsigned char controlField, unsigned char* infoField, int infoFieldLength) {

  frame[0] = FLAG;

  frame[1] = END_SEND; // como so o emissor envia tramas I, assume-se que o campo de endereco e sempre 0x03.

  frame[2] = controlField;

  frame[3] = createBCC(frame[1], frame[2]);

  for(int i = 0; i < infoFieldLength; i++) {
    frame[i + 4] = infoField[i];
  }

  unsigned bcc2 = createBCC_2(infoField, infoFieldLength);

  frame[infoFieldLength + 4] = bcc2;

  frame[infoFieldLength + 5] = FLAG;

  return 0;
}


int sendFrame(unsigned char* frame, int fd, int length) {

    int n;

    if( (n = write(fd, frame, length)) <= 0){
        return -1;
    }

    return n;
}


int readByte(unsigned char* byte, int fd) {

    if(read(fd, byte, sizeof(unsigned char)) <= 0)
        return -1;

    return 0;
}


int readSupervisionFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte) {

    state_machine_st *st = create_state_machine(wantedBytes, wantedBytesLength, addressByte);

    unsigned char byte;

    while(st->state != STOP && finish != 1) {
        if(readByte(&byte, fd) == 0)
          event_handler(st, byte, frame, SUPERVISION);
    }

    int ret = st->foundIndex;

    destroy_st(st);

    if(finish == 1)
      return -1;

    return ret;

}

int readInformationFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte) {

  state_machine_st *st = create_state_machine(wantedBytes, wantedBytesLength, addressByte);
  unsigned char byte;

  while(st->state != STOP) {
      if(readByte(&byte, fd) == 0)
        event_handler(st, byte, frame, INFORMATION);
  }

  // dataLength = length of the data packet sent from the application on the transmitter side
  //              (includes data packet + bcc2, with stuffing)
  int ret = st->dataLength;

  destroy_st(st);

  return ret;
}


int openNonCanonical(char* port, struct termios* oldtio, int vtime, int vmin) {

    int fd = open(port, O_RDWR | O_NOCTTY );
    if (fd <0) {
        perror(port);
        return -1;
    }

    struct termios newtio;

    if ( tcgetattr(fd,oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      return -1;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = vtime;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = vmin;   /* blocking read until 5 chars received */

    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      return -1;
    }

    return fd;
}


int closeNonCanonical(int fd, struct termios* oldtio) {

    sleep(1);

    if (tcsetattr(fd,TCSANOW,oldtio) == -1) {
      perror("tcsetattr");
      return -1;
    }

    return 0;
}


void alarmHandlerInstaller() {
    struct sigaction action;
    action.sa_handler = alarmHandler;

    if(sigemptyset(&action.sa_mask) == -1){
      perror("sigemptyset");
      exit(-1);
    }

    action.sa_flags = 0;

    if(sigaction(SIGALRM, &action, NULL) != 0){
      perror("sigaction");
      exit(-1);
    }
}
