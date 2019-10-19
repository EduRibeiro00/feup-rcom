#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include "macros.h"


struct linkLayer {
    char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
    int baudRate; /*Velocidade de transmissão*/
    unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
    unsigned int timeout; /*Valor do temporizador: 1 s*/
    unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    unsigned char frame[MAX_SIZE_FRAME]; /*Trama*/
    unsigned int frameLength; /*Comprimento atual da trama*/
};

// global variables
struct linkLayer ll;
struct termios oldtio;
int fd;


/**
 * Opens the connection for the receiver
 * @param File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenReceiver(int fd);


/**
 * Opens the connection for the transmitter
 * @param File descriptor for the serial port
 * @return File descriptor; -1 in case of error
 */
int llOpenTransmitter(int fd);


/**
 * Function that opens and establishes the connection between the receiver and the transmitter
 * @param port Port name
 * @param role Flag that indicates the transmitter or the receiver
 * @return File descriptor; -1 in case of error
 */
int llopen(char* port, int role);


/**
 * Function that writes the information contained in the buffer to the serial port
 * @param fd File descriptor of the serial port
 * @param buffer Information to be written
 * @param length Length of the buffer
 * @return Number of characters written; -1 in case of error
 */
int llwrite(int fd, unsigned char* buffer, int length);

/**
 * Function that reads the information written in the serial port
 * @param fd File descriptor of the serial port
 * @param buffer Array of characters where the read information will be stored
 * @return Number of characters read; -1 in case of error
 */
int llread(int fd, unsigned char* buffer);

/**
 * Closes the connection for the receiver
 * @param File descriptor for the serial port
 * @return Positive value when sucess; negative value when error
 */
int llCloseReceiver(int fd);


/**
 * Closes the connection for the transmitter
 * @param File descriptor for the serial port
 * @return Positive value when sucess; negative value when error
 */
int llCloseTransmitter(int fd);


/**
 * Function that closes the connection between the receiver and the transmitter
 * @param File descriptor of the port
 * @param Flag that indicates the transmitter or the receiver
 * @return Positive value when sucess; negative value when error
 */
int llclose(int fd, int role);
