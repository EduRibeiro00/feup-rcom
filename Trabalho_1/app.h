#pragma once


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
int buildDataPacket(char* packetBuffer, int sequenceNumber, char* dataBuffer, int dataLength); 


/**
 * Function that builds a control packet
 * 
 * @param controlByte Can be CTRL_START or CTRL_END, to show if the control packet indicates the beginning or end of the file 
 * @param packetBuffer Buffer that will have the final contents of the packet
 * @fileSize Size of the full file, in bytes
 * @fileName Name of the file
 * @return Length of the packet buffer
 */
int buildControlPacket(unsigned char controlByte, char* packetBuffer, int fileSize, char* fileName);


/**
 * Function, to be called by the sender, that sends a whole file to the receiver
 * 
 * @param fileName Name of the file
 * @param fileSize Size of the file
 * @return 0 if it was sucessful; negative value otherwise
 */
int sendFile(char* fileName, int fileSize);