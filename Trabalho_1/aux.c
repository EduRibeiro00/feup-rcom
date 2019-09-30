#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>

#include "macros.h"
#include "statemachine.c"

unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
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


int sendFrame(unsigned char* frame, int fd) {

    if(write(fd, frame, MAX_SIZE) <= 0)
        return -1;

    return 0;
}


int readByte(unsigned char* byte, int fd) {

    if(read(fd, byte, sizeof(unsigned char)) <= 0)
        return -1;

    return 0;
}



int readSupervisionFrame(unsigned char* frame, int fd) {

    state_machine_st *st = create_state_machine();

    unsigned char byte;

    while(st->state != STOP) {

        if(readByte(&byte, fd) != 0)
            return -1;
        
        event_handler(st, byte, frame);
    }


    return 0;
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

    if ( tcsetattr(fd,TCSANOW,oldtio) == -1) {
      perror("tcsetattr");
      return -1;
    }

    return 0;
}