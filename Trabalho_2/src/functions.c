#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "functions.h"

/**
 * Function that parses the command line arguments, retrieving a struct with all the individual fields
 * 
 * @param args Pointer to the structure that is going to have the individual fields
 * @param commandLineArg Argument from the command line, that is going to be parsed
 * @return int 0 if sucess; -1 otherwise
 */
int parseArguments(struct arguments *args, char *commandLineArg)
{

    printf("lol1\n");
    printf("Parsing command line arguments...\n");
    //verifying FTP protocol
    char *token = strtok(commandLineArg, ":");
    if ((token == NULL) || (strcmp(token, "ftp") != 0))
    {
        printf("-> Error in the protocol name (should be 'ftp')\n");
        return -1;
    }
    // parsing user name
    token = strtok(NULL, ":");
    if (token == NULL || (strlen(token) < 3) || (token[0] != '/') || (token[1] != '/'))
    {
        printf("-> Error parsing the user name\n");
        return -1;
    }
    strcpy(args->user, &token[2]);
    // parsing password
    token = strtok(NULL, "@");
    if (token == NULL || (strlen(token) == 0))
    {
        printf("-> Error parsing the password\n");
        return -1;
    }
    strcpy(args->password, token);
    // parsing hostname
    token = strtok(NULL, "/");
    if (token == NULL || (strlen(token) == 0))
    {
        printf("-> Error parsing the host name\n");
        return -1;
    }
    strcpy(args->host_name, token);
    // parsing file path
    token = strtok(NULL, "\0");
    if (token == NULL || (strlen(token) == 0))
    {
        printf("-> Error parsing the host name\n");
        return -1;
    }

    char filepathAndName[MAX_LENGTH];
   
    strcpy(filepathAndName, token);

    char* token1 = strrchr(filepathAndName, '/');
    if(token1 == NULL){
        strcpy(args->file_name, filepathAndName);
        strcpy(args->file_path, "");
    }
    else{
        strcpy(args->file_name, token1 + 1);
        strncpy(args->file_path, filepathAndName, strlen(filepathAndName)- strlen(args->file_name) - 1);
        args->file_path[strlen(filepathAndName)-strlen(args->file_name) - 1] = '\0';

    }

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
int getIPAdress(char *ipAdress, char *hostName)
{
    struct hostent *h;
    if ((h = gethostbyname(hostName)) == NULL)
    {
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
int createAndConnectSocket(char *address, int port)
{
    int sockfd;
    struct sockaddr_in server_addr;
    // server address handling
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_port = htons(port);
    // open a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        return -1;
    }
    // connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
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
int sendToControlSocket(struct ftp *ftp, char *cmdHeader, char *cmdBody)
{

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
 * 
 * @param sockfd The socket descriptor
 * @param buffer Buffer that is going to store the 3 digit number code received from the server
 * @return int 0 if sucess; -1 otherwise
 */
int receiveFromControlSocket(struct ftp *ftp, char *string, size_t size)
{

    printf("Receiving from control Socket \n");
    FILE *fp = fdopen(ftp->control_socket_fd, "r");

    do
    {
        memset(string, 0, size);
        string = fgets(string, size, fp);
        printf("%s", string);
    } while (!('1' <= string[0] && string[0] <= '5') || string[3] != ' ');

    return 0;
}

int sendComandInterpretResponse(struct ftp *ftp, char *cmdHeader, char *cmdBody, char *response, size_t responseLength)
{

    if (sendToControlSocket(ftp, cmdHeader, cmdBody) < 0)
    {
        printf("Error Sending Comand  %s %s\n", cmdHeader, cmdBody);
        return -1;
    }

    int code;

    while (1)
    {

        receiveFromControlSocket(ftp, response, responseLength);
        code = response[0] - '0';

        switch (code)
        {

        case 1:
            // Expecting another reply
            break;
        case 2:
            // request action sucess
            return 2;
        case 3:
            // Needs aditional information
            return 3;
        case 4:
            //try again
            if (sendToControlSocket(ftp, cmdHeader, cmdBody) < 0)
            {
                printf("Error Sending Comand  %s %s\n", cmdHeader, cmdBody);
                return -1;
            }
            break;
        case 5:
            // error in sending comand, closing control socket , exiting application
            printf("> Comand wasn\'t accepted... \n");
            close(ftp->control_socket_fd);
            exit(-1);
            break;

        default:

            break;
        }
    }
}

int getServerPortForFile(struct ftp *ftp)
{
    char firstByte[4];
    char secondByte[4];
    memset(firstByte, 0, 4);
    memset(secondByte, 0, 4);
    char response[MAX_LENGTH];

    int rtr = sendComandInterpretResponse(ftp, "pasv", "", response, MAX_LENGTH);

    int ipPart1, ipPart2, ipPart3, ipPart4;
    int port1, port2;

    if (rtr == 2)
    {
        // starting process information
        if ((sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ipPart1,
                    &ipPart2, &ipPart3, &ipPart4, &port1, &port2)) < 0)
        {
            printf("ERROR: Cannot process information to calculating port.\n");
            return -1;
        }
    }
    else
    {
        printf("Error receiving pasv command response from server...\n\n");
        return -1;
    }

    char ip[MAX_LENGTH];
    sprintf(ip, "%d.%d.%d.%d", ipPart1, ipPart2, ipPart3, ipPart4);
    int port = port1 * 256 + port2;

    printf("Port number %d\n\n", port);

    if ((ftp->data_socket_fd = createAndConnectSocket(ip, port)) < 0)
    {
        printf("Error creating new socket\n");
        return -1;
    }
    return 0;
}


int retr(struct ftp* ftp, char* fileName){
    
    char response[MAX_LENGTH];

    if(sendComandInterpretResponse(ftp, "RETR", fileName, response, MAX_LENGTH) < 0){
        printf("Error sendimg Comand Retr\n\n");
        return -1;
    }

	return 0;
}

int login(struct ftp *ftp, char *username, char *password)
{

    printf("Sending Username...\n\n");
    char response[MAX_LENGTH];
    int rtr = sendComandInterpretResponse(ftp, "user", username, response, MAX_LENGTH);

    if (rtr == 3)
    {
        printf("Sent Username...\n\n");
    }
    else
    {
        printf("Error sending Username...\n\n");
        return -1;
    }

    printf("Sending Password...\n\n");
    rtr = sendComandInterpretResponse(ftp, "pass", password, response, MAX_LENGTH);

    if (rtr == 2)
    {
        printf("Sent Password...\n\n");
    }
    else
    {
        printf("Error sending Password...\n\n");
        return -1;
    }

    return 0;
}


int changeWorkingDirectory(struct ftp* ftp, char* path){
    
    char response[MAX_LENGTH];

    if(sendComandInterpretResponse(ftp, "CWD", path, response, MAX_LENGTH) < 0){
        printf("Error sendimg Comand CWD\n\n");
        return -1;
    }

	return 0;
}


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

    if(closeFile(fp)<0){
        printf("Error closing file\n");
        return -1;
    }

    close(ftp->data_socket_fd);
    
    return 0;
}

/**
 * Function that opens a file, from its name
 * @param fileName Name of the file to be opened
 * @param mode Mode in which to open the file
 * @return file pointer to the file in question
 */
FILE* openFile(char* fileName, char* mode){


    FILE* fp;
    fp = fopen(fileName, mode);
    
    if (fp == NULL)
    {
        perror(fileName);
        return NULL;
    }
    
    return fp;
}

/**
 * Function that closes a file, from its file pointer
 * @param fp File pointer to the file to be closed
 * @return 0 if successful, EOF if an error occurs
 */
int closeFile(FILE* fp){
    return fclose(fp);
}