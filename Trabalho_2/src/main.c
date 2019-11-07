#include <stdio.h> 
#include "macros.h"
#include "functions.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: ./%s %s", argv[0], "ftp://[<user>:<password>@]<host>/<url-path>");
        return -1;
    }
    // parse command line arguments
    struct arguments args;
    if(parseArguments(&args, argv[1]) != 0) {
        return -1;
    }
    printf("User: %s\n", args.user);
    printf("Password: %s\n", args.password);
    printf("Host name: %s\n", args.host_name);
    printf("File path: %s\n", args.file_path);

    char command[MAX_LENGTH]; // buffer to send commands
    char responseBuffer[MAX_LENGTH]; // buffer to read commands

    // get IP Adress
    char ipAdress[MAX_LENGTH];
    if(getIPAdress(ipAdress, args.host_name) < 0) {
        return -1;
    }
    // create and connect socket to server
    int sockfd;
    if((sockfd = createAndConnectSocket(ipAdress, FTP_PORT_NUMBER)) < 0) {
        return -1;
    }
    receiveFromSocket(sockfd, responseBuffer);
    if(responseBuffer[0] == '2') {
        printf("Expecting username...\n\n");
    }
    // send username
    strcpy(command, "user ");
    strcat(command, args.user);
    strcat(command, "\n");
    sendToSocket(sockfd, command, strlen(command));
    receiveFromSocket(sockfd, responseBuffer);

    return 0;
}