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

    return 0;
}