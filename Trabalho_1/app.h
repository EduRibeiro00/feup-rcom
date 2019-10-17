#pragma once

#include <stdio.h>

struct applicationLayer {
    int fileDescriptor; /*Descritor correspondente à porta série*/
    int status; /*TRANSMITTER | RECEIVER*/
};

struct applicationLayer al;

/**
 * Function that builds an application data packet, receiving a sequence number, and a data buffer
 * containing the bytes to be sent; returns the information on the packet buffer.
 * 
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @param sequenceNumber Sequence number of the packet
 * @param dataBuffer Buffer with the data
 * @param dataLength Length of the data in the buffer
 * @return Length of the packet buffer
 */
int buildDataPacket(unsigned char* packetBuffer, int sequenceNumber, unsigned char* dataBuffer, int dataLength); 


/**
 * Function that builds a control packet
 * 
 * @param controlByte Can be CTRL_START or CTRL_END, to show if the control packet indicates the beginning or end of the file 
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @fileSize Size of the full file, in bytes
 * @fileName Name of the file
 * @return Length of the packet buffer
 */
int buildControlPacket(unsigned char controlByte, unsigned char* packetBuffer, int fileSize, char* fileName);

/**
 * Function, to be called by the reader, that parses the control packets
 * 
 * @param packetBuffer Buffer with the control packet
 * @param fileSize Pointer to the size of the file, to be returned by the function
 * @param fileName Pointer to the name of the file, to be returned by the function
 * @return 0 if it was sucessful; negative value otherwise
 */
int parseControlPacket(unsigned char* packetBuffer, int* fileSize, char* fileName);


/**
 * Function, to be called by the reader, that parses the data packets
 * 
 * @param packetBuffer Buffer with the data packet
 * @param data Pointer to the file data packet extracted, to be returned by the function
 * @param sequenceNumber Pointer to the sequence number of the packet, to be returned by the function
 * @return 0 if it was sucessful; negative value otherwise
 */
int parseDataPacket(unsigned char* packetBuffer, unsigned char* data, int* sequenceNumber);


int sendFile(char *port , char* fileName);

int receiveFile(char *port);