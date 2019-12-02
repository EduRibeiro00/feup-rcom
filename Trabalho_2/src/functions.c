#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include "functions.h"

/**
 * Function that parses the command line arguments, retrieving a struct with all the individual fields
 * 
 * @param args Pointer to the structure that is going to have the individual fields
 * @param commandLineArg Argument from the command line, that is going to be parsed
 * @return int 0 if success; -1 otherwise
 */
int parseArguments(struct arguments *args, char *commandLineArg) {

    printf("Parsing command line arguments...\n");
    
    // verifying FTP protocol
    char *token = strtok(commandLineArg, ":");
    if ((token == NULL) || (strcmp(token, "ftp") != 0)) {
        printf("-> Error in the protocol name (should be 'ftp')\n");
        return -1;
    }

    token = strtok(NULL, "\0");
    char rest_of_string[MAX_LENGTH];
    strcpy(rest_of_string, token);

    // parsing user name
    char aux[MAX_LENGTH];
    strcpy(aux, rest_of_string);
    token = strtok(aux, ":");

    if (token == NULL || (strlen(token) < 3) || (token[0] != '/') || (token[1] != '/')) {
        printf("-> Error parsing the user name\n");
        return -1;
    }
    else if (strcmp(token, rest_of_string) == 0) {
        memset(args->user, 0, sizeof(args->user));
        strcpy(args->user, "anonymous");
        memset(args->password, 0, sizeof(args->password));
        strcpy(args->password, "");

        char aux2[MAX_LENGTH];
        strcpy(aux2, &rest_of_string[2]);
        strcpy(rest_of_string, aux2);
    }
    else {
        memset(args->user, 0, sizeof(args->user));
        strcpy(args->user, &token[2]);
        // parsing password
        token = strtok(NULL, "@");
        if (token == NULL || (strlen(token) == 0)) {
            printf("-> Error parsing the password\n");
            return -1;
        }
        memset(args->password, 0, sizeof(args->password));
        strcpy(args->password, token);

        token = strtok(NULL, "\0");
        strcpy(rest_of_string, token);
    }

    // parsing hostname
    token = strtok(rest_of_string, "/");    
    if (token == NULL) {
        printf("-> Error parsing the hostname\n");
        return -1;
    }
    memset(args->host_name, 0, sizeof(args->host_name));
    strcpy(args->host_name, token);

    // parsing path and name
    token = strtok(NULL, "\0");
    if (token == NULL) {
        printf("-> Error parsing the file path\n");
        return -1;
    }
    char* last = strrchr(token, '/');
    if (last != NULL) {
        memset(args->file_path, 0, sizeof(args->file_path));
        strncpy(args->file_path, token, last - token);
        memset(args->file_name, 0, sizeof(args->file_name));
        strcpy(args->file_name, last + 1);
    }
    else {
        memset(args->file_path, 0, sizeof(args->file_path));
        strcpy(args->file_path, "");
        memset(args->file_name, 0, sizeof(args->file_name));
        strcpy(args->file_name, token);
    }

    printf("Parsed command line arguments.\n\n");

    return 0;

}

/**
 * Function that, having the host name, retrieves the IP address
 * 
 * @param idAddress Variable that is going to point to the IP Address
 * @param hostName The host's name
 * @return int 0 if success; -1 otherwise
 */
int getIPAddress(char *ipAddress, char *hostName) {
    struct hostent *h;
    if ((h = gethostbyname(hostName)) == NULL) {
        herror("gethostbyname");
        return -1;
    }
    strcpy(ipAddress, inet_ntoa(*((struct in_addr *)h->h_addr)));
    return 0;
}

/**
 * Function that creates a new TCP socket, and connects it to the address and port specified
 * 
 * @param address The IP address of the server
 * @param port The number of the port to be used
 * @return int Socket descriptor if success; -1 otherwise
 */
int createAndConnectSocket(char *address, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    // server address handling
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_port = htons(port);
    // open a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        return -1;
    }
    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        return -1;
    }
    return sockfd;
}


/**
 * Function that allows a command to be sent through a socket
 * 
 * @param sockfd The socket descriptor
 * @param command The command to be sent
 * @param commandLength The length of the command
 * @return 0 if success; -1 otherwise
 */
int sendToControlSocket(struct ftp *ftp, char *cmdHeader, char *cmdBody) {
    printf("Sending to control Socket > %s %s\n", cmdHeader, cmdBody);
    int bytes = write(ftp->control_socket_fd, cmdHeader, strlen(cmdHeader));
    if (bytes != strlen(cmdHeader))
        return -1;
    bytes = write(ftp->control_socket_fd, " ", 1);
    if (bytes != 1)
        return -1;
    bytes = write(ftp->control_socket_fd, cmdBody, strlen(cmdBody));
    if (bytes != strlen(cmdBody))
        return -1;
    bytes = write(ftp->control_socket_fd, "\n", 1);
    if (bytes != 1)
        return -1;
    return 0;
}

/**
 * Function that allows the reading of a command through a socket
 * 
 * @param ftp Struct with the socket descriptors
 * @param string Buffer that is going to store the 3 digit number code received from the server
 * @param size Number of digits to be received
 * @return int 0 if success; -1 otherwise
 */
int receiveFromControlSocket(struct ftp *ftp, char *string, size_t size) {
    printf("Receiving from control Socket \n");
    FILE *fp = fdopen(ftp->control_socket_fd, "r");
    do {
        memset(string, 0, size);
        string = fgets(string, size, fp);
        printf("%s", string);
    } while (!('1' <= string[0] && string[0] <= '5') || string[3] != ' ');
    return 0;
}

/**
 * Function that sends a command to the control socket and interprets the response received
 * 
 * @param ftp Struct with the socket descriptors
 * @param cmdHeader Header of the command to be sent
 * @param cmdBody Body of the command to be sent
 * @param response Buffer that is going to store the number code received from the server
 * @param responseLength Number of digits to be received on the response
 * @param readingFile Indicates if the file is about to be read from the data socket
 * @return int Positive (depending on response) if success; -1 otherwise
 */
int sendCommandInterpretResponse(struct ftp *ftp, char *cmdHeader, char *cmdBody, char *response, size_t responseLength, bool readingFile) {
    if (sendToControlSocket(ftp, cmdHeader, cmdBody) < 0) {
        printf("Error Sending Command  %s %s\n", cmdHeader, cmdBody);
        return -1;
    }
    int code;
    while (1) {
        receiveFromControlSocket(ftp, response, responseLength);
        code = response[0] - '0';
        switch (code) {
            case 1:
                // expecting another reply
                if (readingFile) return 2;
                else break;
            case 2:
                // request action success
                return 2;
            case 3:
                // needs aditional information
                return 3;
            case 4:
                // try again
                if (sendToControlSocket(ftp, cmdHeader, cmdBody) < 0) {
                    printf("Error Sending Command  %s %s\n", cmdHeader, cmdBody);
                    return -1;
                }
                break;
            case 5:
                // error in sending command, closing control socket , exiting application
                printf("> Command wasn\'t accepted... \n");
                close(ftp->control_socket_fd);
                exit(-1);
                break;
            default:
                break;
        }
    }
}


/**
 * Function that obtains a server port for the transfer of a file.
 * 
 * @param ftp Struct with the socket descriptors
 * @return int 0 if success; -1 otherwise
 */
int getServerPortForFile(struct ftp *ftp) {
    char firstByte[4];
    char secondByte[4];
    memset(firstByte, 0, 4);
    memset(secondByte, 0, 4);
    char response[MAX_LENGTH];
    int rtr = sendCommandInterpretResponse(ftp, "pasv", "", response, MAX_LENGTH, false);
    int ipPart1, ipPart2, ipPart3, ipPart4;
    int port1, port2;
    if (rtr == 2) {
        // starting to process information
        if ((sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
                    &ipPart1, &ipPart2, &ipPart3, &ipPart4, &port1, &port2)) < 0) {
            printf("ERROR: Cannot process information to calculating port.\n");
            return -1;
        }
    }
    else {
        printf("Error receiving pasv command response from server...\n\n");
        return -1;
    }
    char ip[MAX_LENGTH];
    sprintf(ip, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4);
    int port = port1 * 256 + port2;
    printf("Port number %d\n\n", port);
    if ((ftp->data_socket_fd = createAndConnectSocket(ip, port)) < 0) {
        printf("Error creating new socket\n");
        return -1;
    }
    return 0;
}


/**
 * Function that sends the "retr" command to the control socket, so the file starts being transmited in the file socket
 * 
 * @param ftp Struct containing the socket descriptors
 * @param fileName The name of the file to be transfered
 * @return int 0 if successful; -1 otherwise
 */
int retr(struct ftp* ftp, char* fileName){
    char response[MAX_LENGTH];
    if(sendCommandInterpretResponse(ftp, "RETR", fileName, response, MAX_LENGTH, true) < 0){
        printf("Error sending Command Retr\n\n");
        return -1;
    }
	return 0;
}


/**
 * Function that sends the login information to the socket for authentication
 * 
 * @param ftp Struct that contains the socket descriptors
 * @param username User name to be sent to the socket
 * @param password Password to be sent to the socket
 * @return int 0 if success; -1 otherwise
 */
int login(struct ftp *ftp, char *username, char *password) {
    printf("Sending Username...\n\n");
    char response[MAX_LENGTH];
    int rtr = sendCommandInterpretResponse(ftp, "user", username, response, MAX_LENGTH, false);
    if (rtr == 3) {
        printf("Sent Username...\n\n");
    }
    else {
        printf("Error sending Username...\n\n");
        return -1;
    }
    printf("Sending Password...\n\n");
    rtr = sendCommandInterpretResponse(ftp, "pass", password, response, MAX_LENGTH, false);
    if (rtr == 2) {
        printf("Sent Password...\n\n");
    }
    else {
        printf("Error sending Password...\n\n");
        return -1;
    }
    return 0;
}

/**
 * Function to change the working directory of the FTP, using the CWD command
 * 
 * @param ftp Struct containing the socket descriptors
 * @param path Path of the new working directory to be changed to
 * @return int 0 if successful; -1 otherwise
 */
int changeWorkingDirectory(struct ftp* ftp, char* path) {
    char response[MAX_LENGTH];
    if(sendCommandInterpretResponse(ftp, "CWD", path, response, MAX_LENGTH, false) < 0){
        printf("Error sendimg Comand CWD\n\n");
        return -1;
    }
	return 0;
}


/**
 * Function that downloads a file sent from the server through a socket, and saves it in a local file
 * 
 * @param ftp Struct containing the socket descriptors
 * @param fileName The name of the file to be transfered
 * @return int 0 if successful; -1 otherwise
 */
int downloadFile(struct ftp* ftp, char * fileName){
    FILE *fp = openFile(fileName, "w");
    if (fp == NULL){
        printf("Error opening or creating file\n");
        return -1;
    }
    char buf[1024];
    int bytes;
    printf("Starting to download file with name %s\n", fileName);
    while((bytes = read(ftp->data_socket_fd, buf, sizeof(buf)))){
        if(bytes < 0){
            printf("Error reading from data socket\n");
            return -1;
        }
        if((bytes = fwrite(buf, bytes, 1, fp)) < 0){
            printf("Error writing data to file\n");
            return -1;
        }
    }
    printf("Finished dowloading File\n");
    if(closeFile(fp) < 0){
        printf("Error closing file\n");
        return -1;
    }
    close(ftp->data_socket_fd);
    char response[MAX_LENGTH];
    receiveFromControlSocket(ftp, response, MAX_LENGTH);
    if (response[0] != '2')
        return -1;
    return 0;
}


/**
 * Function that opens a file, from its name
 * 
 * @param fileName Name of the file to be opened
 * @param mode Mode in which to open the file
 * @return File pointer to the file in question; null if error
 */
FILE* openFile(char* fileName, char* mode){
    FILE* fp;
    fp = fopen(fileName, mode);
    if (fp == NULL) {
        perror(fileName);
        return NULL;
    }
    return fp;
}

/**
 * Function that closes a file, from its file pointer
 * 
 * @param fp File pointer to the file to be closed
 * @return 0 if successful, EOF if an error occurs
 */
int closeFile(FILE* fp){
    return fclose(fp);
}


/**
 * Function that ends the connection with the control socket
 * 
 * @param ftp Struct containing the socket descriptors
 * @return int 0 if successful; -1 if error
 */
int disconnectFromSocket(struct ftp* ftp) {
    char response[MAX_LENGTH];
    if(sendCommandInterpretResponse(ftp, "QUIT", "", response, MAX_LENGTH, false) != 2){
        printf("Error sending Command Quit\n\n");
        return -1;
    }
	return 0;
}
