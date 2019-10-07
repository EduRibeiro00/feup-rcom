#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include "data_link.h"
#include "statemachine.h"
#include "aux.h"
#include "app.h"
#include "alarm.h"




/**
 * Opens the connection for the receiver
 * @param File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenReceiver(int fd) {

    if(readSupervisionFrame(ll.frame, fd, SET, END_SEND) == -1)
      return -1;

    printf("Received SET frame\n");

    if(createSupervisionFrame(ll.frame, UA, RECEIVER) != 0)
      return -1;

    // send SET frame to receiver
    if(sendFrame(ll.frame, fd) != 0)
      return -1;

    printf("Sent UA frame\n");

    return fd;
}


/**
 * Opens the connection for the transmitter
 * @param File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenTransmitter(int fd) {

    // creates SET frame
    if(createSupervisionFrame(ll.frame, SET, TRANSMITTER) != 0)
        return -1;

    // send SET frame to receiver
    if(sendFrame(ll.frame, fd) != 0)
        return -1;

    printf("Sent SET frame\n");

    // ----------------------
    // byte stuffing testing

    //  unsigned char bcc2;

    // ll.frame[0] = FLAG;
    // ll.frame[1] = 0x03;
    // ll.frame[2] = 0x00;
    // ll.frame[3] = ll.frame[1] ^ ll.frame[2];
    // ll.frame[4] = 0x7e;
    // ll.frame[5] = 0x7e;
    // ll.frame[6] = 0xfd;
    // ll.frame[7] = 0x7d;
    // bcc2 = ll.frame[4];
    // for(int i = 5; i < 8; i++){
    //   bcc2 = bcc2 ^ ll.frame[i];
    // }
    // ll.frame[8] = bcc2;
    // ll.frame[9] = FLAG;

    // byte_stuffing(ll.frame, 3, 9);


    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(TIMEOUT);

    while (finish != 1) {
      read_value = readSupervisionFrame(ll.frame, fd, UA, END_SEND);

      if(read_value == 0){

        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }


    if(read_value == -1){
      printf("Closing file descriptor\n");
      closeNonCanonical(fd, &oldtio);
      return -1;
    }


    printf("Received UA frame\n");

    return fd;
}

/**
 * Function that opens and establishes the connection between the receiver and the transmitter
 * @param port Port name
 * @param role Flag that indicates the transmitter or the receiver
 * @return File descriptor; -1 in case of error
 */
int llopen(char* port, int role) {

    ll.frame = malloc(sizeof(unsigned char) * (MAX_SIZE));

    // open, in non canonical
    if((fd = openNonCanonical(port, &oldtio, VTIME_VALUE, VMIN_VALUE)) == -1)
      return -1;


    // installs alarm handler
    alarmHandlerInstaller();

    int returnFd;

    if(role == TRANSMITTER) {
        returnFd = llOpenTransmitter(fd);
        if(returnFd < 0) {
          free(ll.frame);
          return -1;
        }
        else return returnFd;

    }
    else if(role == RECEIVER) {
        returnFd = llOpenReceiver(fd);
        if(returnFd < 0) {
          free(ll.frame);
          return -1;
        }
        else return returnFd;
    }

    perror("Invalid role");
    free(ll.frame);
    return -1;
}


/**
 * Closes the connection for the transmitter
 * @param File descriptor for the serial port
 * @return Positive value when sucess; negative value when error
 */
int llCloseTransmitter(int fd) {

    // creates DISC frame
    if(createSupervisionFrame(ll.frame, DISC, TRANSMITTER) != 0)
        return -1;

    // send DISC frame to receiver
    if(sendFrame(ll.frame, fd) != 0)
        return -1;

    printf("Sent DISC frame\n");




    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(TIMEOUT);

    while (finish != 1) {
      read_value = readSupervisionFrame(ll.frame, fd, DISC, END_REC);

      if(read_value == 0){

        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }

    if(read_value == -1){
      printf("Closing file descriptor\n");
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    printf("Received DISC frame\n");




    // creates UA frame
    if(createSupervisionFrame(ll.frame, UA, TRANSMITTER) != 0)
        return -1;

    // send DISC frame to receiver
    if(sendFrame(ll.frame, fd) != 0)
        return -1;

    printf("Sent UA frame\n");

    return 0;
}


/**
 * Closes the connection for the receiver
 * @param File descriptor for the serial port
 * @return Positive value when sucess; negative value when error
 */
int llCloseReceiver(int fd) {

    if(readSupervisionFrame(ll.frame, fd, DISC, END_SEND) == -1)
      return -1;

    printf("Received DISC frame\n");

    // creates DISC frame
    if(createSupervisionFrame(ll.frame, DISC, RECEIVER) != 0)
        return -1;

    // send DISC frame to receiver
    if(sendFrame(ll.frame, fd) != 0)
        return -1;

    printf("Sent DISC frame\n");



    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(TIMEOUT);

    while (finish != 1) {
      read_value = readSupervisionFrame(ll.frame, fd, UA, END_REC);

      if(read_value == 0){

        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }


    if(read_value == -1){
      printf("Closing file descriptor\n");
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    printf("Received UA frame\n");

    return 0;
}


/**
 * Function that closes the connection between the receiver and the transmitter
 * @param File descriptor of the port
 * @param Flag that indicates the transmitter or the receiver
 * @return Positive value when sucess; negative value when error
 */
int llclose(int fd, int role) {

    if(role == TRANSMITTER) {
      if(llCloseTransmitter(fd) < 0) {
        free(ll.frame);
        return -1;
      }
    }
    else if(role == RECEIVER) {
      if(llCloseReceiver(fd) < 0) {
        free(ll.frame);
        return -1;
      }
    }
    else {
      perror("Invalid role");
      free(ll.frame);
      return -1;
    }


    // close, in non canonical
    if(closeNonCanonical(fd, &oldtio) == -1)
      return -1;

    printf("Closed file descriptor\n");
    free(ll.frame);

    return 1;
}
