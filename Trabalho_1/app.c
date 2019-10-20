#include "app.h"
#include "macros.h"
#include "aux.h"
#include "data_link.h"
#include "files.h"


/**
 * Function that builds an application data packet, receiving a sequence number, and a data buffer
 * containing the bytes to be sent; returns the information on the packet buffer.
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @param sequenceNumber Sequence number of the packet
 * @param dataBuffer Buffer with the data
 * @param dataLength Length of the data in the buffer
 * @return Length of the packet buffer
 */
int buildDataPacket(unsigned char *packetBuffer, int sequenceNumber, unsigned char *dataBuffer, int dataLength)
{

    packetBuffer[0] = CTRL_DATA;

    packetBuffer[1] = (unsigned char)sequenceNumber; // basta meter o int?

    int l1, l2;
    convertValueInTwo(dataLength, &l1, &l2);

    packetBuffer[2] = (unsigned char)l2;

    packetBuffer[3] = (unsigned char)l1;

    for (int i = 0; i < dataLength; i++)
        packetBuffer[i + 4] = dataBuffer[i];

    return dataLength + 4;
}

/**
 * Function that builds a control packet
 * @param controlByte Can be CTRL_START or CTRL_END, to show if the control packet indicates the beginning or end of the file 
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @param fileSize Size of the full file, in bytes
 * @param fileName Name of the file
 * @return Length of the packet buffer
 */
int buildControlPacket(unsigned char controlByte, unsigned char *packetBuffer, int fileSize, char *fileName)
{

    packetBuffer[0] = controlByte;

    packetBuffer[1] = TYPE_FILESIZE;

    int length = 0;
    int currentFileSize = fileSize;

    // cicle to separate file size (v1) in bytes
    while (currentFileSize > 0)
    {
        int rest = currentFileSize % 256;
        int div = currentFileSize / 256;
        length++;

        // shifts all bytes to the right, to make space for the new byte
        for (unsigned int i = 2 + length; i > 3; i--)
            packetBuffer[i] = packetBuffer[i - 1];

        packetBuffer[3] = (unsigned char)rest;

        currentFileSize = div;
    }

    packetBuffer[2] = (unsigned char)length;

    packetBuffer[3 + length] = TYPE_FILENAME;

    int fileNameStart = 5 + length; // beginning of v2

    packetBuffer[4 + length] = (unsigned char)(strlen(fileName) + 1); // adds file name length (including '\0)

    for (unsigned int j = 0; j < (strlen(fileName) + 1); j++)
    { // strlen(fileName) + 1 in order to add the '\0' char
        packetBuffer[fileNameStart + j] = fileName[j];
    }

    return 3 + length + 2 + strlen(fileName) + 1; // total length of the packet
}

/**
 * Function, to be called by the reader, that parses the data packets
 * @param packetBuffer Buffer with the data packet
 * @param data Pointer to the file data packet extracted, to be returned by the function
 * @param sequenceNumber Pointer to the sequence number of the packet, to be returned by the function
 * @return 0 if it was sucessful; negative value otherwise
 */
int parseDataPacket(unsigned char *packetBuffer, unsigned char *data, int *sequenceNumber)
{

    if (packetBuffer[0] != CTRL_DATA)
    {
        return -1;
    }

    *sequenceNumber = (int)packetBuffer[1];

    int size_of_data;

    size_of_data = convertValueInOne((int)packetBuffer[3], (int)packetBuffer[2]);

    for (int i = 0; i < size_of_data; i++)
    {
        data[i] = packetBuffer[i + 4];
    }

    return 0;
}

/**
 * Function, to be called by the reader, that parses the control packets
 * @param packetBuffer Buffer with the control packet
 * @param fileSize Pointer to the size of the file, to be returned by the function
 * @param fileName Pointer to the name of the file, to be returned by the function
 * @return 0 if it was sucessful; negative value otherwise
 */
int parseControlPacket(unsigned char *packetBuffer, int *fileSize, char *fileName)
{

    if (packetBuffer[0] != CTRL_START && packetBuffer[0] != CTRL_END)
    {
        return -1;
    }

    int length1;

    if (packetBuffer[1] == TYPE_FILESIZE)
    {

        *fileSize = 0;
        length1 = (int)packetBuffer[2];

        for (int i = 0; i < length1; i++)
        {
            *fileSize = *fileSize * 256 + (int)packetBuffer[3 + i];
        }
    }
    else
    {
        return -1;
    }

    int length2;
    int fileNameStart = 5 + length1;

    if (packetBuffer[fileNameStart - 2] == TYPE_FILENAME)
    {

        length2 = (int)packetBuffer[fileNameStart - 1];

        for (int i = 0; i < length2; i++)
        {
            fileName[i] = packetBuffer[fileNameStart + i];
        }
    }
    else
    {
        return -1;
    }

    return 0;
}


/**
 * Function to receive a file, sent through the serial port
 * @param port Name of the serial port
 * @return 0 if successful; negative if an error occurs
 */
int receiveFile(char *port)
{

    // fills appLayer fields
    al.status = RECEIVER;

    if ((al.fileDescriptor = llopen(port, al.status)) <= 0) 
    {
        return -1;
    }

    printf("\n---------------llopen done---------------\n\n");

    unsigned char packetBuffer[MAX_PACK_SIZE];

    int packetSize;
    int fileSize;
    unsigned char data[MAX_DATA_SIZE];
    char fileName[255];

    packetSize = llread(al.fileDescriptor, packetBuffer);

    if (packetSize < 0)
    {
        return -1;
    }

    // if start control packet was received
    if (packetBuffer[0] == CTRL_START)
    {
        if (parseControlPacket(packetBuffer, &fileSize, fileName) < 0)
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }

    FILE *fp = openFile(fileName, "w");
    if (fp == NULL)
        return -1;

    int expectedSequenceNumber = 0;

    // starts received data packets (file data)
    while (1)
    {

        packetSize = llread(al.fileDescriptor, packetBuffer);

        if (packetSize < 0)
        {
            return -1;
        }

        // received data packet
        if (packetBuffer[0] == CTRL_DATA)
        {
            int sequenceNumber;

            if (parseDataPacket(packetBuffer, data, &sequenceNumber) < 0)
                return -1;

            // if sequence number doesn't match
            if (expectedSequenceNumber != sequenceNumber)
            {
                printf("Sequence number does not match!\n");
                return -1;
            }

            expectedSequenceNumber = (expectedSequenceNumber + 1) % 256;

            int dataLength = packetSize - 4;
            
            // writes to the file the content read from the serial port
            if (fwrite(data, sizeof(unsigned char), dataLength, fp) != dataLength)
            {
                return -1;
            }
        }
        // received end packet; file was fully transmitted
        else if (packetBuffer[0] == CTRL_END)
        {
            break;
        }
    }

    if (getFileSize(fp) != fileSize)
    {
        printf("file size does not match\n");
        return -1;
    }

    int fileSizeEnd;
    char fileNameEnd[255];

    if (parseControlPacket(packetBuffer, &fileSizeEnd, fileNameEnd) < 0)
    {
        return -1;
    }

    if((fileSize != fileSizeEnd) || (strcmp(fileNameEnd, fileName) != 0)){
        printf("Information in start and end packets does not match");
        return -1;
    }

    // close, in non canonical
    if (llclose(al.fileDescriptor, al.status) < 0)
        return -1;

    printf("\n---------------llclose done---------------\n\n");

    return 0;
}


/**
 * Function to send a file, using the serial port, to its destination
 * @param port Name of the serial port
 * @param fileName Name of the file to be sent
 * @return 0 if successful; negative if an error occurs
 */
int sendFile(char *port, char *fileName) {

    FILE *fp = openFile(fileName, "r");
    if (fp == NULL){
        printf("Cannot find the file to transmit\n");
        return -1;
    }

    // fills appLayer fields
    al.status = TRANSMITTER;


    if ((al.fileDescriptor = llopen(port, al.status)) <= 0)
    {
        return -1;
    }

    printf("\n---------------llopen done---------------\n\n");


    unsigned char packetBuffer[MAX_PACK_SIZE];
    int fileSize = getFileSize(fp);

    int packetSize = buildControlPacket(CTRL_START, packetBuffer, fileSize, fileName);

    // sends control start packet, to indicate the start of the file transfer
    if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0)
    {
        closeFile(fp);
        return -1;
    }

    printf("\n---------------STARTING TO SEND FILE---------------\n\n");

    unsigned char data[MAX_DATA_SIZE];
    int length_read;
    int sequenceNumber = 0;

    while (1)
    {
        // reads a data chunk from the file
        length_read = fread(data, sizeof(unsigned char), MAX_DATA_SIZE, fp);

        if (length_read != MAX_DATA_SIZE)
        {
            if (feof(fp))
            {

                packetSize = buildDataPacket(packetBuffer, sequenceNumber, data, length_read);
                sequenceNumber = (sequenceNumber + 1) % 256;
                
                // sends the last data frame
                if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0)
                {
                    closeFile(fp);
                    return -1;
                }

                break;
            }
            else
            {
                perror("error reading file data");
                return -1;
            }
        }

        packetSize = buildDataPacket(packetBuffer, sequenceNumber, data, length_read);
        sequenceNumber = (sequenceNumber + 1) % 256;
    
        // sends a data frame
        if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0)
        {
            closeFile(fp);
            return -1;
        }
       
    }

    packetSize = buildControlPacket(CTRL_END, packetBuffer, fileSize, fileName);

    // sends control end packet; indicating the end of the file transfer
    if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0)
    {
        closeFile(fp);
        return -1;
    }

    printf("\n---------------ENDED SENDING FILE---------------\n\n");

    if (llclose(al.fileDescriptor, al.status) < 0)
        return -1;

    printf("\n---------------llclose done---------------\n\n");

    if (closeFile(fp) != 0)
        return -1;

    return 0;
}
