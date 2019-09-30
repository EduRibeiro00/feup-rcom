#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "macros.h"

unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
}


int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role) {

    frame[0] = FLAG;

    if(role == TRANSMITTER) {
        if(controlField == SET || controlField == DISC) {
            frame[1] = END_SEND;
        }
        else if(controlField == UA || controlField == RR || controlField == REJ) {
            frame[1] = END_REC;
        }
        else return -1;
    }
    else if(role == RECEIVER) {
        if(controlField == SET || controlField == DISC) {
            frame[1] = END_REC;
        }
        else if(controlField == UA || controlField == RR || controlField == REJ) {
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

    write(fd, frame, MAX_SIZE);
}