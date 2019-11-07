#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "functions.h"


/**
 * Function that parses the command line arguments, retrieving a struct with all the individual fields
 * 
 * @param args Pointer to the structure that is going to have the individual fields
 * @param commandLineArg Argument from the command line, that is going to be parsed
 * @return int 0 if sucess; -1 otherwise
 */
int parseArguments(struct arguments* args, char* commandLineArg) {
    printf("Parsing command line arguments...\n");
    //verifying FTP protocol
    char* token = strtok(commandLineArg, ":");
    if((token == NULL) || (strcmp(token, "ftp") != 0)) {
        printf("-> Error in the protocol name (should be 'ftp')\n");
        return -1;
    }
    // parsing user name
    token = strtok(NULL, ":");
    if(token == NULL || (strlen(token) < 3) || (token[0] != '/') || (token[1] != '/')) {
        printf("-> Error parsing the user name\n");
        return -1;
    }
    strcpy(args->user, &token[2]);
    // parsing password
    token = strtok(NULL, "@");
    if(token == NULL || (strlen(token) == 0)) {
        printf("-> Error parsing the password\n");
        return -1;
    }
    strcpy(args->password, token);
    // parsing hostname
    token = strtok(NULL, "/");
    if(token == NULL || (strlen(token) == 0)) {
        printf("-> Error parsing the host name\n");
        return -1;
    }
    strcpy(args->host_name, token);
    // parsing file path
    token = strtok(NULL, "\0");
    if(token == NULL || (strlen(token) == 0)) {
        printf("-> Error parsing the host name\n");
        return -1;
    }
    strcpy(args->file_path, token);
    printf("Parsed command line arguments.\n\n");
    return 0;
}


/**
 * Function that, having the host name, retrieves the IP address
 * 
 * @param idAdress Variable that is going to point to the IP Adress
 * @param hostName The host's name
 * @return int 0 if sucess; -1 otherwise
 */
int getIPAdress(char* ipAdress, char* hostName) {
    struct hostent *h;
    if ((h=gethostbyname(hostName)) == NULL) {  
        herror("gethostbyname");
        return -1;
    }
    strcpy(ipAdress, inet_ntoa(*((struct in_addr *)h->h_addr)));
    return 0;
}


/**
 * Function that creates a new TCP socket, and connects it to the address and port specified
 * 
 * @param address The IP address of the server
 * @param port The number of the port to be used
 * @return int Socket descriptor if sucess; -1 otherwise
 */
int createAndConnectSocket(char* address, int port) {
    int	sockfd;
	struct sockaddr_in server_addr;
    // server address handling
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(address);
	server_addr.sin_port = htons(port);
	// open a TCP socket
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    	perror("socket()");
        return -1;
    }
	// connect to the server
    if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
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
 * @return int Number of bytes written if success; -1 otherwise
 */
int sendToSocket(int sockfd, char* command, int commandLength) {
    printf("Sending %s\n", command);
    int bytes = write(sockfd, command, commandLength);
    if(bytes != commandLength)
        return -1;
    else
        return bytes;
}


/**
 * Function that allows the reading of a command through a socket
 * 
 * @param sockfd The socket descriptor
 * @param buffer Buffer that is going to store the 3 digit number code received from the server
 * @return int 0 if sucess; -1 otherwise
 */
int receiveFromSocket(int sockfd, char* buffer) {
    readState state = READING_NUMBER_CODE;
    int index = 0;
    char currentChar;
    printf("\n");
    while(state != DONE) {
        read(sockfd, &currentChar, 1);
        printf("%c", currentChar);
        switch(state) {
            //reads 3 digit number code
            case READING_NUMBER_CODE:
                if(currentChar == ' ') {
                    if(index != 3) {
                        printf("\n-------------------\n Error receiving response code\n-------------------\n\n");
                        return -1;
                    }
                    index = 0;
                    state = READING_MESSAGE;
                }
                else {
                    if(currentChar == '-') {
                        index = 0;
                        state = READING_REST_OF_LINE;
                    }
                    else if(isdigit(currentChar)) {
                        buffer[index] = currentChar;
                        index++;
                    }
                }
                break;
            // reads until end of line
            case READING_MESSAGE:
                if(currentChar == '\n') {
                    state = DONE;
                }
                break;
            // reads and ignores rest of line. After reading newline character, goes to a new line
            case READING_REST_OF_LINE:
                if(currentChar == '\n') {
                    state = READING_NEW_LINE;
                }
                break;
            // reads the beginning of a new line, and checks if it can be the final line or not
            case READING_NEW_LINE:
                if(isdigit(currentChar) && (currentChar == buffer[0])) {
                    index = 1;
                    state = READING_RESPONSE_CODE;
                }
                else {
                    state = READING_REST_OF_LINE;
                }
                break;
            // reading the response code, checking if it is equal to the first 3 number code received
            case READING_RESPONSE_CODE:
                if(currentChar == buffer[index]) {
                    index++;
                }
                else if(index == 3) {
                    if(currentChar == ' ') {
                        state = DONE;
                    }
                    else {
                        index = 0;
                        state = READING_REST_OF_LINE;
                    }
                }
                else {
                    index = 0;
                    state = READING_REST_OF_LINE;
                }
                break;
            default:
                break;
        }
    }
    printf("\n\n");
}
