#pragma once

#include <termios.h>
#include <unistd.h>

unsigned char createBCC(unsigned char a, unsigned char c);


unsigned char createBCC_2(unsigned char* frame, int length);


int byte_stuffing(int length);


int byte_destuffing(int length);


int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);


int createInformationFrame(unsigned char* frame, unsigned char controlField, unsigned char* infoField, int infoFieldLength);


int readSupervisionFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte);


int readInformationFrame(unsigned char* frame, int fd, unsigned char* wantedBytes, int wantedBytesLength, unsigned char addressByte);


int sendFrame(unsigned char* frame, int fd, int length);


int readByte(unsigned char* byte, int fd);


int openNonCanonical(char* port, struct termios* oldtio, int vtime, int vmin);


int closeNonCanonical(int fd, struct termios* oldtio);


void alarmHandlerInstaller();

// ------------------------------

void convertValueInTwo(int k, int* l1, int* l2);


int convertValueInOne(int l1, int l2);