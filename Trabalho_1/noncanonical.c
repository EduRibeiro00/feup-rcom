/*Non-Canonical Input Processing*/
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
#include "aux.h"
#include "data_link.h"
#include "app.h"


int main(int argc, char** argv)
{
    printf("estou aqui no main\n");
    int fd;

    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS2", argv[1])!=0) &&
          (strcmp("/dev/ttyS3", argv[1])!=0) &&
          (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    if(receiveFile(argv[1])<0){
      return -1;
    }
    return 0;
}

