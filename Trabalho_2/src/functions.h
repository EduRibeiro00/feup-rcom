#pragma once

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h> 
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include "macros.h"

/**
 * Struct that contains the necessary fields to parse the command line arguments passed
 */
struct arguments {
    char user[MAX_LENGTH]; /**< user string */
    char password[MAX_LENGTH]; /**< password string */
    char host_name[MAX_LENGTH]; /**< host name string */
    char file_path[MAX_LENGTH]; /**< file path string */    
};

/**
 * Enum representing the different states when reading a server response from socket
 */
typedef enum {
    READING_NUMBER_CODE, /**< reading the first 3 digit number code */
    READING_MESSAGE, /**< when response has a single line, reads and ignores the rest of the message */
    READING_NEW_LINE, /**< when response has multiple lines, and we are at the beginning of a new line */
    READING_REST_OF_LINE, /**< when response has multiple lines, when line does not begin with a number, ignore it */
    READING_RESPONSE_CODE, /**< reading the second 3 digit number code, that must be equal to the first one */
    DONE /**< when reading is done */
} readState;


/**
 * Function that parses the command line arguments, retrieving a struct with all the individual fields
 * 
 * @param args Pointer to the structure that is going to have the individual fields
 * @param commandLineArg Argument from the command line, that is going to be parsed
 * @return int 0 if success; -1 otherwise
 */
int parseArguments(struct arguments* args, char* commandLineArg);


/**
 * Function that, having the host name, retrieves the IP address
 * 
 * @param idAdress Variable that is going to point to the IP Adress
 * @param hostName The host's name
 * @return int 0 if success; -1 otherwise
 */
int getIPAdress(char* ipAdress, char* hostName);


/**
 * Function that creates a new TCP socket, and connects it to the address and port specified
 * 
 * @param address The IP address of the server
 * @param port The number of the port to be used
 * @return int Socket descriptor if success; -1 otherwise
 */
int createAndConnectSocket(char* address, int port);


/**
 * Function that allows a command to be sent through a socket
 * 
 * @param sockfd The socket descriptor
 * @param command The command to be sent
 * @param commandLength The length of the command
 * @return int Number of bytes written if success; -1 otherwise
 */
int sendToSocket(int sockfd, char* command, int commandLength);


/**
 * Function that allows the reading of a command through a socket
 * 
 * @param sockfd The socket descriptor
 * @param buffer Buffer that is going to store the 3 digit number code received from the server
 * @return int 0 if sucess; -1 otherwise
 */
int receiveFromSocket(int sockfd, char* buffer);


// TODO: function to, after sending a command, to interpret the response from the server