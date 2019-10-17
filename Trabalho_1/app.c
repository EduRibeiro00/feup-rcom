#include "app.h"
#include "macros.h"
#include "aux.h"
#include "data_link.h"
#include "files.h"

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
    int expectedSequenceNumber = 0;

    packetSize = llread(al.fileDescriptor, packetBuffer);

    if (packetSize < 0)
    {
        return -1;
    }

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

     

    while (1)
    {
        packetSize = llread(al.fileDescriptor, packetBuffer);
        printf("estou a entrar no parse data\n");
        if (packetSize < 0)
        {
            return -1;
        }
        if (packetBuffer[0] == CTRL_DATA)
        {

            int sequenceNumber;
            printf("estou a entrar no parse data\n");
            if (parseDataPacket(packetBuffer, data, &sequenceNumber) < 0)
                return -1;

            if (expectedSequenceNumber++ != sequenceNumber)
            {
                printf("Sequence number does not match!\n");
                return -1;
            }
            int dataLength = packetSize - 4;
            if (fwrite(data, sizeof(unsigned char), dataLength, fp) != dataLength)
            {
                return -1;
            }
        }
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

    if(fileSize != fileSizeEnd || !strcmp(fileNameEnd, fileName)){
        printf("iinformation in start and end packets doesnt match");
        return -1;
    }
    

    // close, in non canonical
    if (llclose(al.fileDescriptor, RECEIVER) < 0)
        return -1;

    printf("\n---------------llclose done---------------\n\n");

    return 0;
}

int sendFile(char *port, char *fileName)
{

    FILE *fp = openFile(fileName, "r");
    if (fp == NULL)
        return -1;

    // fills appLayer fields
    al.status = TRANSMITTER;

    unsigned char packetBuffer[MAX_PACK_SIZE];

    int fileSize = getFileSize(fp);

    if ((al.fileDescriptor = llopen(port, al.status)) <= 0)
    {
        return -1;
    }

    printf("\n---------------llopen done---------------\n\n");

    int packetSize = buildControlPacket(CTRL_START, packetBuffer, fileSize, fileName);

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
        length_read = fread(data, sizeof(unsigned char), MAX_DATA_SIZE, fp);
        if (length_read != MAX_DATA_SIZE)
        {
            if (feof(fp))
            {
                packetSize = buildDataPacket(packetBuffer, sequenceNumber, data, length_read);
                sequenceNumber = (sequenceNumber + 1) % 256;
                //manda a ultima trama de dados

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
        //manda a ultima trama de dados

        if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0)
        {
            closeFile(fp);
            return -1;
        }

       
    }

    packetSize = buildControlPacket(CTRL_END, packetBuffer, fileSize, fileName);

    if (llwrite(al.fileDescriptor, packetBuffer, packetSize) < 0)
    {
        closeFile(fp);
        return -1;
    }

    printf("\n---------------ENDED SENDING FILE---------------\n\n");

    if (llclose(al.fileDescriptor, TRANSMITTER) < 0)
        return -1;

    printf("\n---------------llclose done---------------\n\n");

    if (closeFile(fp) != 0)
        return -1;

    return 0;
}
