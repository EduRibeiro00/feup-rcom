#pragma once

#include <termios.h>
#include <unistd.h>


/**
 * Function to create the Block Check Character relative to the Address and Control fields
 * @param a Address Character of the frame
 * @param c Control Character of the frame
 * @return Expected value for the Block Check Character
 */
unsigned char createBCC(unsigned char a, unsigned char c);


/**
 * Function to create the Block Check Character relative to the Data Characters of the frame
 * @param frame Frame position where the Data starts
 * @param length Number of Data Characters to process
 * @return Expected value for the Block Check Character
 */
unsigned char createBCC_2(unsigned char* frame, int length);


/**
 * Function to apply byte stuffing to the Data Characters of a frame
 * @param frame Address of the frame
 * @param length Number of Data Characters to process
 * @return Length of the new frame, post byte stuffing
 */
int byteStuffing(unsigned char* frame, int length);


/**
 * Function to reverse the byte stuffing applied to the Data Characters of a frame
 * @param frame Address of the frame
 * @param length Number of Data Characters to process
 * @return Length of the new frame, post byte destuffing
 */
int byteDestuffing(unsigned char* frame, int length);


/**
 * Function to create a supervision frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param role Role for which to create the frame, marking the difference between the Transmitter and the Receiver
 * @return 0 if successful; negative if an error occurs
 */
int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);


/**
 * Function to create an information frame for the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param controlField Control field of the supervision frame
 * @param infoField Start address of the information to be inserted into the information frame
 * @param infoFieldLength Number of data characters to be inserted into the information frame
 * @return Returns 0, as there is no place at which an error can occur
 */
int createInformationFrame(unsigned char* frame, unsigned char controlField, unsigned char* infoField, int infoFieldLength);


/**
 * Function to read a supervision frame, sent according to the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param fd File descriptor from which to read the frame
 * @param wantedBytes Array containing the possible expected control bytes of the frame
 * @param wantedBytesLength Number of possible expected control bytes of the frame
 * @param addressByte Address from which a frame is expected
 * @return Index of the wanted byte found, in the wantedBytes array
 */
int readSupervisionFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte);


/**
 * Function to read an information frame, sent according to the serial port file transfer protocol
 * @param frame Address where the frame will be stored
 * @param fd File descriptor from which to read the frame
 * @param wantedBytes Array containing the possible expected control bytes of the frame
 * @param wantedBytesLength Number of possible expected control bytes of the frame
 * @param addressByte Address from which a frame is expected
 * @return Length of the data packet sent, including byte stuffing and BCC2
 */
int readInformationFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte);


/**
 * Function to send a frame to the designated file descriptor
 * @param frame Start address of the frame to the sent
 * @param fd File descriptor to which to write the information
 * @param length Size of the frame to be sent (size of information to be written)
 * @return Number of bytes written if successful; negative if an error occurs
 */
int sendFrame(unsigned char* frame, int fd, int length);


/**
 * Function to read a byte from the designated file descriptor
 * @param byte Address to which to store the byte
 * @param fd File descriptor from which to read the byte
 * @return Return value of the read() call if successful; negative if an error occurs
 */
int readByte(unsigned char* byte, int fd);


/**
 * Function to open the file descriptor through which to execute the serial port communications,
 * in the non-canonical mode, according to the serial port file transfer protocol
 * @param port Name of the port to be opened
 * @param oldtio Struct where the pre-open port settings will be stored
 * @param vtime Value to be assigned to the VTIME field of the new settings - time between bytes read
 * @param vmin Value to be assigned to the VMIN field of the new settings - minimum amount of bytes to read
 * @return File descriptor that was opened with the given port
 */
int openNonCanonical(char* port, struct termios* oldtio, int vtime, int vmin);


/**
 * Function to close the file descriptor through which the serial port communications were executed
 * @param fd File descriptor where the port has been opened
 * @param oldtio Struct containing the original port settings have been saved, so they can be restored
 * @return 0 if successful; negative if an error occurs
 */
int closeNonCanonical(int fd, struct termios* oldtio);


/**
 * Function to install the alarm handler, using sigaction
 */
void alarmHandlerInstaller();

// ------------------------------

/**
 * Auxiliary function to convert a decimal value into two (max. 8 bits) values, for hexadecimal representation
 * @param k Decimal value to be converted
 * @param l1 Least significant bits of the converted value
 * @param l2 Most significant bits of the converted value
 */
void convertValueInTwo(int k, int* l1, int* l2);


/**
 * Auxiliary function to convert two (max. 8 bits) values, from hexadecimal representation, into one single decimal
 * @param l1 Least significant bits of the value to be converted
 * @param l2 Most significant bits of the value to be converted
 * @return Decimal converted value
 */
int convertValueInOne(int l1, int l2);
