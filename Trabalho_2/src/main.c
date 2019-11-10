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
    printf("File name: %s\n", args.file_name);

    struct ftp ftp;

    char command[MAX_LENGTH]; // buffer to send commands
    char responseBuffer[MAX_LENGTH]; // buffer to read commands

    // get IP Adress
    char ipAdress[MAX_LENGTH];
    if(getIPAdress(ipAdress, args.host_name) < 0) {
        return -1;
    }
    // create and connect socket to server
    if((ftp.control_socket_fd = createAndConnectSocket(ipAdress, FTP_PORT_NUMBER)) < 0) {
        printf("Error creating new socket\n");
        return -1;
    }
    
    receiveFromControlSocket(&ftp, responseBuffer, MAX_LENGTH);
    
    if(responseBuffer[0] == '2') {
        printf("Expecting username...\n\n");
    }
    else{
        printf("Error in conection...\n\n");
        return -1;
    }

    if(login(&ftp, args.user, args.password)<0){
        printf("Login failed...\n\n");
        return -1;
    }

    if(changeWorkingDirectory(&ftp, args.file_path)<0){
        printf("Error changing directory\n");
        return -1;
    }

    if(getServerPortForFile(&ftp)<0){
        printf("Error getting server Port for file\n");
        return -1;
    }




    return 0;
}