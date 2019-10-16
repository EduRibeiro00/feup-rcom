#include "app.h"
#include "macros.h"
#include "aux.h"
#include "data_link.h"

int buildDataPacket(char* packetBuffer, int sequenceNumber, char* dataBuffer, int dataLength) {

    packetBuffer[0] = CTRL_DATA;

    packetBuffer[1] = (unsigned char) sequenceNumber; // basta meter o int?

    int l1, l2;
    convertValueInTwo(dataLength, &l1, &l2);

    packetBuffer[2] = (unsigned char) l2;

    packetBuffer[3] = (unsigned char) l1;

    for(int i = 0; i < dataBuffer; i++)
        packetBuffer[i + 4] = dataBuffer[i];

    return dataBuffer + 4;
}


int buildControlPacket(unsigned char controlByte, char* packetBuffer, int fileSize, char* fileName) {

    packetBuffer[0] = controlByte;

    packetBuffer[1] = TYPE_FILESIZE;

    int length = 0;
    int currentFileSize = fileSize;

    // cicle to separate file size (v1) in bytes
    while(currentFileSize > 0) {
        int rest = currentFileSize % 256;
        int div = currentFileSize / 256;
        length++;

        // shifts all bytes to the right, to make space for the new byte
        for (unsigned int i = 2 + length; i > 3; i++)
            packetBuffer[i] = packetBuffer[i-1];


        packetBuffer[3] = (unsigned char) rest;

        currentFileSize = div;
    }

    packetBuffer[2] = (unsigned char) length;

    packetBuffer[2 + length + 1] = TYPE_FILENAME;

    int fileNameStart = 2 + length + 3; // beginning of v2

    packetBuffer[2 + length + 2] = (unsigned char) (strlen(fileName) + 1); // adds file name length (including '\0)

    for(unsigned int j = 0; j < (strlen(fileName) + 1); j++) { // strlen(fileName) + 1 in order to add the '\0' char
        packetBuffer[fileNameStart + j] = fileName[j];
    }


    return 3 + length + 2 + strlen(fileName) + 1; // total length of the packet
}