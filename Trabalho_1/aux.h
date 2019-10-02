#pragma once

unsigned char createBCC(unsigned char a, unsigned char c);


int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);


// returns supervision control byte
int readSupervisionFrame(unsigned char* frame, int fd);


int sendFrame(unsigned char* frame, int fd);


int readByte(unsigned char* byte, int fd);


int openNonCanonical(char* port, struct termios* oldtio, int vtime, int vmin);


int closeNonCanonical(int fd, struct termios* oldtio);
