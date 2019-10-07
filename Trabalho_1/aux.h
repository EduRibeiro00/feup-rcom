#pragma once

unsigned char createBCC(unsigned char a, unsigned char c);


unsigned char createBCC_2(unsigned char* frame, unsigned char start, unsigned char end);


int byte_stuffing(unsigned char* frame, int start, int end);


int byte_destuffing(unsigned char* frame, int start, int end);


int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);


int readSupervisionFrame(unsigned char* frame, int fd, unsigned char wantedByte, unsigned char addressByte);


int sendFrame(unsigned char* frame, int fd);


int readByte(unsigned char* byte, int fd);


int openNonCanonical(char* port, struct termios* oldtio, int vtime, int vmin);


int closeNonCanonical(int fd, struct termios* oldtio);


void alarmHandlerInstaller();
