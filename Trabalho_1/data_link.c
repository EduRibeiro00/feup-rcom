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

    unsigned char wantedByte[1];
    wantedByte[0] = SET;
    if(readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_SEND) == -1)
      return -1;

    printf("Received SET frame\n");

    if(createSupervisionFrame(ll.frame, UA, RECEIVER) != 0)
      return -1;

    // send SET frame to receiver
    if(sendFrame(ll.frame, fd) == -1)
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
    if(sendFrame(ll.frame, fd) == -1)
        return -1;

    printf("Sent SET frame\n");

    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(TIMEOUT);

    unsigned char wantedByte[1];
    wantedByte[0] = UA;

    while (finish != 1) {
      read_value = readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_SEND);

      if(read_value >= 0){

        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }


    if(read_value == -1){
      printf("Closing file descriptor\n");
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
          closeNonCanonical(fd, &oldtio);
          return -1;
        }
        else return returnFd;

    }
    else if(role == RECEIVER) {
        returnFd = llOpenReceiver(fd);
        if(returnFd < 0) {
          free(ll.frame);
          closeNonCanonical(fd, &oldtio);
          return -1;
        }
        else return returnFd;
    }

    perror("Invalid role");
    free(ll.frame);
    closeNonCanonical(fd, &oldtio);
    return -1;
}


/**
 * Function that writes the information contained in the buffer to the serial port
 * @param fd File descriptor of the serial port
 * @param buffer Information to be written
 * @param length Length of the buffer
 * @return Number of characters written; -1 in case of error
 */
int llwrite(int fd, char* buffer, int length) {

  static unsigned char controlByte = S_0;

  if (createInformationFrame(ll.frame, controlByte, buffer, length) != 0) {
    free(ll.frame);
    closeNonCanonical(fd, &oldtio);
    return -1;
  }

  if(byte_stuffing(ll.frame, DATA_START, length) != 0){
    free(ll.frame);
    closeNonCanonical(fd, &oldtio);
    return -1;
  }

  int numWritten;
  if((numWritten = sendFrame(ll.frame, fd)) == -1)
    return -1;

  printf("Sent I frame\n");


  int read_value = -1;
  finish = 0;
  num_retr = 0;

  alarm(TIMEOUT);

  unsigned char wantedBytes[2];

  while (finish != 1) {

    if (controlByte == S_0) {
      wantedBytes[0] = RR_1;
      wantedBytes[1] = REJ_0;
    }
    else if (controlByte == S_1) {
      wantedBytes[0] = RR_0;
      wantedBytes[1] = REJ_1;
    }

    read_value = readSupervisionFrame(ll.frame, fd, wantedBytes, 2, END_SEND);

    if(read_value == 0) { // read_value é o índice do wantedByte que foi encontrado
      // Cancels alarm
      alarm(0);
      finish = 1;
    }

  }


  if(read_value != 0){
    printf("Closing file descriptor\n");
    return -1;
  }

  printf("Received RR frame\n");

  if (controlByte == S_0)
    controlByte = S_1;
  else if (controlByte == S_1)
    controlByte = S_0;
  else return -1;

  return numWritten;

}


/**
 * Function that reads the information written in the serial port
 * @param fd File descriptor of the serial port
 * @param buffer Array of characters where the read information will be stored
 * @return Number of characters read; -1 in case of error
 */
int llread(int fd, char* buffer) {

  static int controlVal = 0;

  unsigned char wantedBytes[2];
  wantedBytes[0] = S_0;
  wantedBytes[1] = S_1;

  int read_value;
  unsigned char responseByte;

  if((read_value = readInformationFrame(ll.frame, fd, wantedBytes, 2, END_SEND)) == -1) {
    if (controlVal == 0) {
      responseByte = REJ_1;
    }
    else responseByte = REJ_0;
  }
  else {
    if (controlVal == 0) {
      responseByte = RR_1;
    }
    else responseByte = RR_0;
  }

  printf("Received I frame\n");

  if(createSupervisionFrame(ll.frame, responseByte, RECEIVER) != 0)
    return -1;

  // send SET frame to receiver
  if(sendFrame(ll.frame, fd) == -1)
    return -1;

  printf("Sent ACK frame\n");

  return read_value;

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
    if(sendFrame(ll.frame, fd) == -1)
        return -1;

    printf("Sent DISC frame\n");




    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(TIMEOUT);

    unsigned char wantedByte[1];
    wantedByte[0] = DISC;

    while (finish != 1) {
      read_value = readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_REC);

      if(read_value >= 0){

        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }

    if(read_value == -1){
      printf("Closing file descriptor\n");
      return -1;
    }

    printf("Received DISC frame\n");




    // creates UA frame
    if(createSupervisionFrame(ll.frame, UA, TRANSMITTER) != 0)
        return -1;

    // send DISC frame to receiver
    if(sendFrame(ll.frame, fd) == -1)
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

    unsigned char wantedByte[1];
    wantedByte[0] = DISC;

    if(readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_SEND) == -1)
      return -1;

    printf("Received DISC frame\n");

    // creates DISC frame
    if(createSupervisionFrame(ll.frame, DISC, RECEIVER) != 0)
        return -1;

    // send DISC frame to receiver
    if(sendFrame(ll.frame, fd) == -1)
        return -1;

    printf("Sent DISC frame\n");



    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(TIMEOUT);

    wantedByte[0] = UA;

    while (finish != 1) {
      read_value = readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_REC);

      if(read_value >= 0){

        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }


    if(read_value == -1){
      printf("Closing file descriptor\n");
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
        closeNonCanonical(fd, &oldtio);
        return -1;
      }
    }
    else if(role == RECEIVER) {
      if(llCloseReceiver(fd) < 0) {
        free(ll.frame);
        closeNonCanonical(fd, &oldtio);
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
