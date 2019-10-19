#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdbool.h>
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
int llOpenReceiver(int fd)
{
  unsigned char wantedByte[1];
  wantedByte[0] = SET;
  if (readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_SEND) == -1)
    return -1;

  printf("Received SET frame\n");

  if (createSupervisionFrame(ll.frame, UA, RECEIVER) != 0)
    return -1;

  ll.frameLength = BUF_SIZE_SUP;

  // send SET frame to receiver
  if (sendFrame(ll.frame, fd, ll.frameLength) == -1)
    return -1;

  printf("Sent UA frame\n");

  return fd;
}

/**
 * Opens the connection for the transmitter
 * @param File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenTransmitter(int fd)
{
  unsigned char responseBuffer[BUF_SIZE_SUP]; // buffer to read the response 


  // creates SET frame
  if (createSupervisionFrame(ll.frame, SET, TRANSMITTER) != 0)
    return -1;

  ll.frameLength = BUF_SIZE_SUP;

  // send SET frame to receiver
  if (sendFrame(ll.frame, fd, ll.frameLength) == -1)
    return -1;

  printf("Sent SET frame\n");


  int read_value = -1;
  finish = 0;
  num_retr = 0;
  resendFrame = false;

  alarm(ll.timeout);

  unsigned char wantedByte[1];
  wantedByte[0] = UA;

  while (finish != 1)
  {
    read_value = readSupervisionFrame(responseBuffer, fd, wantedByte, 1, END_SEND);
    if (resendFrame)
    {
      sendFrame(ll.frame, fd, ll.frameLength);
      resendFrame = false;
    }

    if (read_value >= 0)
    {
      // Cancels alarm
      alarm(0);
      finish = 1;
    }
  }

  if (read_value == -1)
  {
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
int llopen(char *port, int role)
{

  strcpy(ll.port, port);
  ll.baudRate = BAUDRATE;
  ll.numTransmissions = NUM_RETR;
  ll.timeout = TIMEOUT;
  ll.sequenceNumber = 0;

  int fd;

  // open, in non canonical
  if ((fd = openNonCanonical(port, &oldtio, VTIME_VALUE, VMIN_VALUE)) == -1)
    return -1;

  // installs alarm handler
  alarmHandlerInstaller();

  int returnFd;

  if (role == TRANSMITTER)
  {
    returnFd = llOpenTransmitter(fd);
    if (returnFd < 0)
    {
      closeNonCanonical(fd, &oldtio);
      return -1;
    }
    else
      return returnFd;
  }
  else if (role == RECEIVER)
  {
    returnFd = llOpenReceiver(fd);
    if (returnFd < 0)
    { 
      closeNonCanonical(fd, &oldtio);
      return -1;
    }
    else
      return returnFd;
  }

  perror("Invalid role");
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
int llwrite(int fd, unsigned char *buffer, int length)
{
  unsigned char responseBuffer[BUF_SIZE_SUP]; // buffer to receive the response

  unsigned char controlByte;
  if (ll.sequenceNumber == 0)
    controlByte = S_0;
  else
    controlByte = S_1;

  if (createInformationFrame(ll.frame, controlByte, buffer, length) != 0)
  {
    closeNonCanonical(fd, &oldtio);
    return -1;
  }

  int fullLength; // frame length after stuffing

  if ((fullLength = byteStuffing(ll.frame, length)) < 0)
  {
    closeNonCanonical(fd, &oldtio);
    return -1;
  }

  ll.frameLength = fullLength;
  int numWritten;

  bool dataSent = false;

  while (!dataSent)
  {
    if ((numWritten = sendFrame(ll.frame, fd, ll.frameLength)) == -1)
    {
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    printf("Sent I frame\n");

    int read_value = -1;
    finish = 0;
    num_retr = 0;

    alarm(ll.timeout);

    unsigned char wantedBytes[2];

    if (controlByte == S_0)
    {
      wantedBytes[0] = RR_1;
      wantedBytes[1] = REJ_0;
    }
    else if (controlByte == S_1)
    {
      wantedBytes[0] = RR_0;
      wantedBytes[1] = REJ_1;
    }

    while (finish != 1)
    {
      read_value = readSupervisionFrame(responseBuffer, fd, wantedBytes, 2, END_SEND);

      if (resendFrame)
      {
        sendFrame(ll.frame, fd, ll.frameLength);
        resendFrame = false;
      }

      if (read_value >= 0)
      { // read_value é o índice do wantedByte que foi encontrado
        // Cancels alarm
        alarm(0);
        finish = 1;
      }
    }

    if (read_value == -1)
    {
      printf("Closing file descriptor\n");
      
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    if (read_value == 0) // read a RR
      dataSent = true;
    else // read a REJ
      dataSent = false;

    printf("Received response frame (%x)\n", responseBuffer[2]);
  }

  if (ll.sequenceNumber == 0)
    ll.sequenceNumber = 1;
  else if (ll.sequenceNumber == 1)
    ll.sequenceNumber = 0;
  else
    return -1;


  return (numWritten - 6); // length of the data packet length sent to the receiver
}

/**
 * Function that reads the information written in the serial port
 * @param fd File descriptor of the serial port
 * @param buffer Array of characters where the read information will be stored
 * @return Number of characters read; -1 in case of error
 */
int llread(int fd, unsigned char *buffer)
{

  int numBytes;
  unsigned char wantedBytes[2];
  wantedBytes[0] = S_0;
  wantedBytes[1] = S_1;

  int read_value;

  bool isBufferFull = false;

  while (!isBufferFull)
  {

    read_value = readInformationFrame(ll.frame, fd, wantedBytes, 2, END_SEND);

    printf("Received I frame\n");


    if ((numBytes = byteDestuffing(ll.frame, read_value)) < 0)
    {
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    int controlByteRead;
    if (ll.frame[2] == S_0)
      controlByteRead = 0;
    else if (ll.frame[2] == S_1)
      controlByteRead = 1;

    unsigned char responseByte;
    if (ll.frame[numBytes - 2] == createBCC_2(&ll.frame[DATA_START], numBytes - 6))
    { // if bcc2 is correct

      if (controlByteRead != ll.sequenceNumber)
      { // duplicated trama; discard information

        // ignora dados da trama
        if (controlByteRead == 0)
        {
          responseByte = RR_1;
          ll.sequenceNumber = 1;
        }
        else
        {
          responseByte = RR_0;
          ll.sequenceNumber = 0;
        }
      }
      else
      { // new trama

        // passes information to the buffer
        for (int i = 0; i < numBytes - 6; i++)
        {
          buffer[i] = ll.frame[DATA_START + i];
        }

        isBufferFull = true;

        if (controlByteRead == 0)
        {
          responseByte = RR_1;
          ll.sequenceNumber = 1;
        }
        else
        {
          responseByte = RR_0;
          ll.sequenceNumber = 0;
        }
      }
    }
    else
    { // if bcc2 is not correct
      if (controlByteRead != ll.sequenceNumber)
      { // duplicated trama

        // ignores frame data

        if (controlByteRead == 0)
        {
          responseByte = RR_1;
          ll.sequenceNumber = 1;
        }
        else
        {
          responseByte = RR_0;
          ll.sequenceNumber = 0;
        }
      }
      else
      { // new trama

        // ignores frame data, because of error

        if (controlByteRead == 0)
        {
          responseByte = REJ_0;
          ll.sequenceNumber = 0;
        }
        else
        {
          responseByte = REJ_1;
          ll.sequenceNumber = 1;
        }
      }
    }


    if (createSupervisionFrame(ll.frame, responseByte, RECEIVER) != 0)
    {
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    ll.frameLength = BUF_SIZE_SUP;

    // send RR/REJ frame to receiver
    if (sendFrame(ll.frame, fd, ll.frameLength) == -1)
    {
      closeNonCanonical(fd, &oldtio);
      return -1;
    }

    printf("Sent response frame (%x)\n", ll.frame[2]);

  }

  return (numBytes - 6); // number of bytes of the data packet read
}

/**
 * Closes the connection for the transmitter
 * @param File descriptor for the serial port
 * @return Positive value when sucess; negative value when error
 */
int llCloseTransmitter(int fd)
{
  unsigned char responseBuffer[BUF_SIZE_SUP]; // buffer to receive the response

  ll.frameLength = BUF_SIZE_SUP;

  // creates DISC frame
  if (createSupervisionFrame(ll.frame, DISC, TRANSMITTER) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(ll.frame, fd, ll.frameLength) == -1)
    return -1;

  printf("Sent DISC frame\n");

  int read_value = -1;
  finish = 0;
  num_retr = 0;

  alarm(ll.timeout);

  unsigned char wantedByte[1];
  wantedByte[0] = DISC;

  while (finish != 1)
  {
    read_value = readSupervisionFrame(responseBuffer, fd, wantedByte, 1, END_REC);

    if (resendFrame)
    {
      sendFrame(ll.frame, fd, ll.frameLength);
      resendFrame = false;
    }

    if (read_value >= 0)
    {
      // Cancels alarm
      alarm(0);
      finish = 1;
    }
  }

  if (read_value == -1)
  {
    printf("Closing file descriptor\n");
    return -1;
  }

  printf("Received DISC frame\n");

  // creates UA frame
  if (createSupervisionFrame(ll.frame, UA, TRANSMITTER) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(ll.frame, fd, ll.frameLength) == -1)
    return -1;

  printf("Sent UA frame\n");

  return 0;
}

/**
 * Closes the connection for the receiver
 * @param File descriptor for the serial port
 * @return Positive value when sucess; negative value when error
 */
int llCloseReceiver(int fd)
{
  unsigned char responseBuffer[BUF_SIZE_SUP]; // buffer to receive the response

  ll.frameLength = BUF_SIZE_SUP;

  unsigned char wantedByte[1];
  wantedByte[0] = DISC;

  if (readSupervisionFrame(ll.frame, fd, wantedByte, 1, END_SEND) == -1)
    return -1;

  printf("Received DISC frame\n");

  // creates DISC frame
  if (createSupervisionFrame(ll.frame, DISC, RECEIVER) != 0)
    return -1;

  // send DISC frame to receiver
  if (sendFrame(ll.frame, fd, ll.frameLength) == -1)
    return -1;

  printf("Sent DISC frame\n");

  int read_value = -1;
  finish = 0;
  num_retr = 0;

  alarm(ll.timeout);

  wantedByte[0] = UA;

  while (finish != 1)
  {
    read_value = readSupervisionFrame(responseBuffer, fd, wantedByte, 1, END_REC);

    if (resendFrame)
    {
      sendFrame(ll.frame, fd, ll.frameLength);
      resendFrame = false;
    }

    if (read_value >= 0)
    {
      // Cancels alarm
      alarm(0);
      finish = 1;
    }
  }

  if (read_value == -1)
  {
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
int llclose(int fd, int role)
{

  if (role == TRANSMITTER)
  {
    if (llCloseTransmitter(fd) < 0)
    {
      closeNonCanonical(fd, &oldtio);
      return -1;
    }
  }
  else if (role == RECEIVER)
  {
    if (llCloseReceiver(fd) < 0)
    { 
      closeNonCanonical(fd, &oldtio);
      return -1;
    }
  }
  else
  {
    perror("Invalid role");
    return -1;
  }

  // close, in non canonical
  if (closeNonCanonical(fd, &oldtio) == -1)
    return -1;

  if (close(fd) != 0)
    return -1;

  return 1;
}
