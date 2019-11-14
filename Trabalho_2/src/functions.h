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
    char file_name[MAX_LENGTH]; /**< file name string */ 
};

/**
 * Struct that contains the control and data file descriptors for the FTP
 */
struct ftp {
    int control_socket_fd; /**< file descriptor to control socket */
    int data_socket_fd; /**< file descriptor to data socket */
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
 * @param idAdress Variable that is going to point to the IP Address
 * @param hostName The host's name
 * @return int 0 if success; -1 otherwise
 */
int getIPAddress(char* ipAddress, char* hostName);


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
 * @param ftp Struct with the socket descriptors
 * @param cmdHeader Header of the command to be sent
 * @param cmdBody Body of the command to be sent
 * @return int Number of bytes written if success; -1 otherwise
 */
int sendToControlSocket(struct ftp* ftp, char* cmdHeader, char* cmdBody);


/**
 * Function that allows the reading of a command through a socket
 * 
 * @param ftp Struct with the socket descriptors
 * @param string Buffer that is going to store the number code received from the server
 * @param size Number of digits to be received
 * @return int 0 if success; -1 otherwise
 */
int receiveFromControlSocket(struct ftp *ftp, char* string, size_t size);


/**
 * Function that sends a command to the control socket and interprets the response received
 * 
 * @param ftp Struct with the socket descriptors
 * @param cmdHeader Header of the command to be sent
 * @param cmdBody Body of the command to be sent
 * @param response Buffer that is going to store the number code received from the server
 * @param responseLength Number of digits to be received on the response
 * @return int Positive (depending on response) if success; -1 otherwise
 */
int sendCommandInterpretResponse(struct ftp* ftp, char* cmdHeader,  char* cmdBody, char* response, size_t responseLength);


/**
 * Function that sends the login information to the socket for authentication
 * 
 * @param ftp Struct that contains the socket descriptors
 * @param username User name to be sent to the socket
 * @param password Password to be sent to the socket
 * @return int 0 if success; -1 otherwise
 */
int login(struct ftp* ftp, char* username, char* password);


// TODO: function to, after sending a command, to interpret


/**
 * Function that obtains a server port for the transfer of a file.
 * 
 * @param ftp Struct with the socket descriptors
 * @return int 0 if success; -1 otherwise
 */
int getServerPortForFile(struct ftp *ftp);


/**
 * Function to change the working directory of the FTP, using the CWD command
 * 
 * @param ftp Struct containing the socket descriptors
 * @param path Path of the new working directory to be changed to
 * @return int 0 if successful; -1 otherwise
 */
int changeWorkingDirectory(struct ftp* ftp, char* path);

int downloadFile(struct ftp* ftp, char* fileName);

FILE* openFile(char* fileName, char* mode);

int closeFile(FILE* fp);

int retr(struct ftp* ftp, char* fileName);