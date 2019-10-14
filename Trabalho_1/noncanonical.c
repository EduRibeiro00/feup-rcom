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

    // fills linkLayer fields
    strcpy(ll.port, argv[1]);
    ll.baudRate = BAUDRATE;
    ll.numTransmissions = NUM_RETR;
    ll.timeout = TIMEOUT;
    ll.sequenceNumber = 0;


    // fills appLayer fields
    al.status = RECEIVER;


    if((al.fileDescriptor = llopen(ll.port, al.status)) <= 0){
      return -1;
    }

    printf("\n---------------llopen done---------------\n\n");

    char buffer[20];
    int numRead;

    if((numRead = llread(al.fileDescriptor, buffer)) < 0) {
      printf("correu mal :(\n");
      return -1;
    }

    printf("\n\n\n");

    for(int i = 0; i < numRead; i++)
      printf("%c", buffer[i]);


    printf("\n%d\n\n", numRead);


    if((numRead = llread(al.fileDescriptor, buffer)) < 0) {
      printf("correu mal :(\n");
      return -1;
    }


    for(int i = 0; i < numRead; i++)
      printf("%c", buffer[i]);

    printf("\n%d\n\n", numRead);

    if((numRead = llread(al.fileDescriptor, buffer)) < 0) {
      printf("correu mal :(\n");
      return -1;
    }

    for(int i = 0; i < numRead; i++)
      printf("%c", buffer[i]);

    printf("\n%d\n\n", numRead);



    // close, in non canonical
    if(llclose(al.fileDescriptor, RECEIVER) < 0)
      return -1;


    printf("\n---------------llclose done---------------\n\n");


    close(al.fileDescriptor);
    return 0;
}
