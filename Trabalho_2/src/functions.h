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
int sendCommand(int sockfd, char* command, int commandLength);


/**
 * Function that allows the reading of a command through a socket
 * 
 * @param sockfd The socket descriptor
 * @param buffer Buffer that is going to store the command received
 * @param bufferLength The length of the buffer
 * @return int Number of bytes read if success; -1 otherwise
 */
int receiveCommand(int sockfd, char* buffer, int bufferLength);
