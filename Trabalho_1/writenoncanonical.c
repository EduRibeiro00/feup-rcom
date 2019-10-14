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


    if ( (argc < 2) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS2", argv[1])!=0) &&
          (strcmp("/dev/ttyS3", argv[1])!=0) &&
          (strcmp("/dev/ttyS4", argv[1])!=0 ))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    // fills linkLayer fields
    strcpy(ll.port, argv[1]);
    ll.baudRate = BAUDRATE;
    ll.numTransmissions = NUM_RETR;
    ll.timeout = TIMEOUT;
    ll.sequenceNumber = 0;

    // fills appLayer fields
    al.status = TRANSMITTER;


    if((al.fileDescriptor = llopen(ll.port, al.status))<= 0){
      return -1;
    }

    printf("\n---------------llopen done---------------\n\n");

    char buffer[20];
    buffer[0] = 'o';
    buffer[1] = 'l';
    buffer[2] = 'a';
    buffer[3] = '!';
    buffer[4] = ':';
    buffer[5] = ')';

  
    if(llwrite(al.fileDescriptor, buffer, 6) < 0) {
      printf("deu erro");
      return -1;
    }

    buffer[2] = 'e';
    buffer[6] = 'f';
    buffer[7] = 'i';
    buffer[8] = 'g';
    buffer[9] = 'a';

    if(llwrite(al.fileDescriptor, buffer, 10) < 0) {
      printf("deu erro");
      return -1;
    }

    buffer[10] = 'c';
    buffer[11] = 'r';
    buffer[12] = 'l';
    buffer[13] = 'h';

    if(llwrite(al.fileDescriptor, buffer, 14) < 0) {
      printf("deu erro");
      return -1;
    }
    
    if(llclose(al.fileDescriptor, TRANSMITTER) < 0)
      return -1;


    printf("\n---------------llclose done---------------\n\n");

    close(al.fileDescriptor);
    return 0;
}
