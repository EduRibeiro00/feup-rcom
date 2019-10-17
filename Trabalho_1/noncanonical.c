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
          (strcmp("/dev/ttyS4", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }

    // // fills appLayer fields
    // al.status = RECEIVER;

    // if ((al.fileDescriptor = llopen(argv[1], al.status)) <= 0)
    // {
    //     return -1;
    // }

    // unsigned char dataBuffer[20];
    // unsigned char packetBuffer[MAX_DATA_SIZE];
    // int packetLength;
    // int sequenceNumber;
    // int dataLength;

    // if((packetLength = llread(al.fileDescriptor, packetBuffer)) < 0)
    //   return -1;

    // for(int i = 0; i < packetLength; i++) {
    //   printf("%x\n", packetBuffer[i]);
    // }

    // if(parseDataPacket(packetBuffer, dataBuffer, &sequenceNumber) < 0)
    //   return -1;

    // printf("Packet length: %d\n", packetLength);
    // printf("Sequence number: %d\n", sequenceNumber);
    // printf("Data length: %d", packetLength - 4);

    // for(int i = 0; i < packetLength - 4; i++) {
    //   printf("%x\n", dataBuffer[i]);
    // }

    if(receiveFile(argv[1])<0){
      return -1;
    }

    return 0;
}

