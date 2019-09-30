#pragma once

#define MAX_SIZE 255

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0
#define RECEIVER    1


#define NUM_RETR    3
#define TIMEOUT     3


#define FLAG     0x7E
#define END_SEND 0x03
#define END_REC  0x01
#define SET      0x02
#define DISC     0x0B
#define UA       0x07
#define RR       0xFF // (mudar estes
#define REJ      0xFF //  valores)