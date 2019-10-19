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
#include "files.h"


int main(int argc, char** argv)
{

    if ( (argc < 3) ||
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) &&
  	      (strcmp("/dev/ttyS1", argv[1])!=0) &&
          (strcmp("/dev/ttyS2", argv[1])!=0) &&
          (strcmp("/dev/ttyS3", argv[1])!=0) &&
          (strcmp("/dev/ttyS4", argv[1])!=0 ))) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
  
    // // fills appLayer fields
    // al.status = TRANSMITTER;

    // if ((al.fileDescriptor = llopen(argv[1], al.status)) <= 0)
    // {
    //     return -1;
    // }

    // unsigned char dataBuffer[20];
    // dataBuffer[0] = 'o';
    // dataBuffer[1] = 'l';
    // dataBuffer[2] = 'a';
    // dataBuffer[3] = 'o';
    // dataBuffer[4] = 'l';
    // dataBuffer[5] = 'e';
    // dataBuffer[6] = '1';
    // dataBuffer[7] = '2';
    // dataBuffer[8] = '3';

    // unsigned char packetBuffer[MAX_DATA_SIZE];
    // int packetLength;
    // int sequenceNumber = 0;
    // int dataLength = 9;


    // printf("Sequence number: %d\n", sequenceNumber);
    // printf("Data length: %d", dataLength);

    // if((packetLength = buildDataPacket(packetBuffer, sequenceNumber, dataBuffer, dataLength)) < 0)
    //   return -1;

    // printf("Packet length: %d\n", packetLength);

    // for(int i = 0; i < packetLength; i++) {
    //   printf("%x\n", packetBuffer[i]);
    // }

    // if((packetLength = llwrite(al.fileDescriptor, packetBuffer, packetLength)) < 0)
    //   return -1;

    if(sendFile(argv[1], argv[2])<0){
      return -1;
    }
    
    return 0;
}
