#pragma once

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>


unsigned char createBCC(unsigned char a, unsigned char c);


int createSupervisionFrame(unsigned char* frame, unsigned char controlField, int role);


int sendFrame(unsigned char* frame, int fd);