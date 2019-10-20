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


/**
 * Function to create the Block Check Character relative to the Address and Control fields
 * @param a Address Character of the frame
 * @param c Control Character of the frame
 * @return Expected value for the Block Check Character
 */
unsigned char createBCC(unsigned char a, unsigned char c) {
    return a ^ c;
}


/**
 * Function to create the Block Check Character relative to the Data Characters of the frame
 * @param frame Frame position where the Data starts
 * @param length Number of Data Characters to process
 * @return Expected value for the Block Check Character
 */
unsigned char createBCC_2(unsigned char* frame, int length) {

  unsigned char bcc2 = frame[0];

  for(int i = 1; i < length; i++){
    bcc2 = bcc2 ^ frame[i];
  }

  return bcc2;
}


/**
 * Function to apply byte stuffing to the Data Characters of a frame
 * @param frame Frame position where the Data starts
 * @param length Number of Data Characters to process
 * @return Length of the new Data section, post byte stuffing
 */
int byteStuffing(unsigned char* frame, int length) {

  // allocates space for auxiliary buffer (length of the packet, plus 6 bytes for the frame header and tail)
  unsigned char aux[length + 6];

  for(int i = 0; i < length + 6 ; i++){
    aux[i] = frame[i];
  }


  // passes information from the frame to aux
  
  int finalLength = DATA_START;
  // parses aux buffer, and fills in correctly the frame buffer
  for(int i = DATA_START; i < (length + 6); i++){

    if(aux[i] == FLAG && i != (length + 5)) {
      frame[finalLength] = ESCAPE_BYTE;
      frame[finalLength+1] = BYTE_STUFFING_FLAG;
      finalLength = finalLength + 2;
    }
    else if(aux[i] == ESCAPE_BYTE && i != (length + 5)) {
      frame[finalLength] = ESCAPE_BYTE;
      frame[finalLength+1] = BYTE_STUFFING_ESCAPE;
      finalLength = finalLength + 2;
    }
    else{
      frame[finalLength] = aux[i];
      finalLength++;
    }
  }

  return finalLength;
}


/**
 * Function to reverse the byte stuffing applied to the Data Characters of a frame
 * @param frame Frame position where the Data starts
 * @param length Number of Data Characters to process
 * @return Length of the new Data section, post byte destuffing
 */
int byteDestuffing(unsigned char* frame, int length) {

  // allocates space for the maximum possible frame length read (length of the data packet + bcc2, already with stuffing, plus the other 5 bytes in the frame)
  unsigned char aux[length + 5];

  // copies the content of the frame (with stuffing) to the aux frame
  for(int i = 0; i < (length + 5) ; i++) {
    aux[i] = frame[i];
  }

  int finalLength = DATA_START;

  // iterates through the aux buffer, and fills the frame buffer with destuffed content
  for(int i = DATA_START; i < (length + 5); i++) {

    if(aux[i] == ESCAPE_BYTE){
      if (aux[i+1] == BYTE_STUFFING_ESCAPE) {
        frame[finalLength] = ESCAPE_BYTE;
      }
      else if(aux[i+1] == BYTE_STUFFING_FLAG) {
        frame[finalLength] = FLAG;
      }
      i++;
      finalLength++;
    }
    else{
      frame[finalLength] = aux[i];
      finalLength++;
    }
  }

  return finalLength;
}


/**
 * Function to create a supervision frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param role Role for which to create the frame, marking the difference between the Transmitter and the Receiver
 * @return 0 if successful; negative if an error occurs
 */
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


/**
 * Function to create an information frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param infoField Start address of the information to be inserted into the information frame
 * @param infoFieldLength Number of data characters to be inserted into the information frame
 * @return Returns 0, as there is no place at which an error can occur
 */
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


/**
 * Function to send a frame to the designated file descriptor
 * @param frame Start address of the frame to the sent
 * @param fd File descriptor to which to write the information
 * @param length Size of the frame to be sent (size of information to be written)
 * @return Number of bytes written if successful; negative if an error occurs
 */
int sendFrame(unsigned char* frame, int fd, int length) {

    int n;

    if( (n = write(fd, frame, length)) <= 0){
        return -1;
    }

    return n;
}


/**
 * Function to read a byte from the designated file descriptor
 * @param byte Address to which to store the byte
 * @param fd File descriptor from which to read the byte
 * @return Return value of the read() call if successful; negative if an error occurs
 */
int readByte(unsigned char* byte, int fd) {

    if(read(fd, byte, sizeof(unsigned char)) <= 0)
        return -1;

    return 0;
}


/**
 * Function to read a supervision frame, sent according to the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param fd File descriptor from which to read the frame
 * @param wantedBytes Array containing the possible expected control bytes of the frame
 * @param wantedBytesLength Number of possible expected control bytes of the frame
 * @param addressByte Address from which a frame is expected
 * @return Index of the wanted byte found, in the wantedBytes array
 */
int readSupervisionFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte) {

    state_machine_st *st = create_state_machine(wantedBytes, wantedBytesLength, addressByte);

    unsigned char byte;

    while(st->state != STOP && finish != 1 && !resendFrame) {
        if(readByte(&byte, fd) == 0)
          event_handler(st, byte, frame, SUPERVISION);
    }

    int ret = st->foundIndex;

    destroy_st(st);

    if(finish == 1 || resendFrame)
      return -1;

    return ret;

}


/**
 * Function to read an information frame, sent according to the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param fd File descriptor from which to read the frame
 * @param wantedBytes Array containing the possible expected control bytes of the frame
 * @param wantedBytesLength Number of possible expected control bytes of the frame
 * @param addressByte Address from which a frame is expected
 * @return Length of the data packet sent, including byte stuffing and BCC2
 */
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


/**
 * Function to open the file descriptor through which to execute the serial port communications,
 * in the non-canonical mode, according to the serial port file transfer protocol
 * @param port Name of the port to be opened
 * @param oldtio Struct where the pre-open port settings will be stored
 * @param vtime Value to be assigned to the VTIME field of the new settings - time between bytes read
 * @param vmin Value to be assigned to the VMIN field of the new settings - minimum amount of bytes to read
 * @return File descriptor that was opened with the given port
 */
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


/**
 * Function to close the file descriptor through which the serial port communications were executed
 * @param fd File descriptor where the port has been opened
 * @param oldtio Struct containing the original port settings have been saved, so they can be restored
 * @return 0 if successful; negative if an error occurs
 */
int closeNonCanonical(int fd, struct termios* oldtio) {

    sleep(1);

    if (tcsetattr(fd,TCSANOW,oldtio) == -1) {
      perror("tcsetattr");
      return -1;
    }

    return 0;
}


/**
 * Function to install the alarm handler, using sigaction
 */
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


// ------------------------------

/**
 * Auxiliary function to convert a decimal value into two (max. 8 bits) values, for hexadecimal representation
 * @param k Decimal value to be converted
 * @param l1 Least significant bits of the converted value
 * @param l2 Most significant bits of the converted value
 */
void convertValueInTwo(int k, int* l1, int* l2) {
  *l1 = k % 256;
  *l2 = k / 256;
}


/**
 * Auxiliary function to convert two (max. 8 bits) values, from hexadecimal representation, into one single decimal
 * @param l1 Least significant bits of the value to be converted
 * @param l2 Most significant bits of the value to be converted
 * @return Decimal converted value
 */
int convertValueInOne(int l1, int l2) {
  return 256 * l2 + l1;
}
