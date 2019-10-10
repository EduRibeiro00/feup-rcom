#ifndef _MACROS_H_
#define _MACROS_H_

#define MAX_SIZE 255

#define BAUDRATE B4800 //38400 is the normal value
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define TRANSMITTER 0
#define RECEIVER    1

#define SUPERVISION 0
#define INFORMATION 1

#define NUM_RETR    3
#define TIMEOUT     3

#define FLAG     0x7E
#define END_SEND 0x03
#define END_REC  0x01
#define S_0      0x00
#define S_1      0x40
#define SET      0x03
#define DISC     0x0B
#define UA       0x07
#define RR_0     0X05
#define RR_1     0x85
#define REJ_0    0x01
#define REJ_1    0x81
#define VTIME_VALUE    0
#define VMIN_VALUE     1

#define BYTE_STUFFING_ESCAPE 0x5D
#define BYTE_STUFFING_FLAG 0x5E
#define ESCAPE_BYTE 0x7D

#define BUF_SIZE_SUP  5

#define DATA_START    4

#endif
