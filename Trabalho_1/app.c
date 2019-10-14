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

    while(1) {
        int rest = currentFileSize % 256;
        int div = currentFileSize / 256;
        length++;
    
        // if value already fits in 1 byte, no need to subdivide more
        if(div < 1)
            break;

        packetBuffer[2 + length] = (unsigned char) rest;
        currentFileSize = rest;
    }

    packetBuffer[2] = (unsigned char) length;

    packetBuffer[2 + length + 1] = TYPE_FILENAME;

    int curPos = 2 + length + 3;
    int nameSize = 0;

    while(fileName[nameSize] != '\0') {
        packetBuffer[curPos] = fileName[nameSize]; // adds, char by char, the file name to the packet
        nameSize++;
        curPos++;
    }

    packetBuffer[curPos] = fileName[nameSize]; // adds the '\0' char
    packetBuffer[2 + length + 2] = (unsigned char) (nameSize + 1); // adds file name length (including '\0)


    return 6 + length + nameSize; // full length of the packet buffer
}