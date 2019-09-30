/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "macros.h"
#include "aux.h"
#include "data_link.h"
#include "app.h"

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
    char buf[255];
    char buf1[255];
    
    int i, sum = 0, speed = 0;
    
    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }
  
  
    // open, in non canonical
    if((fd = openNonCanonical(argv[1], &oldtio, VTIME, VMIN)) == -1)
      return -1;
  

    




    // close, in non canonical
    if(closeNonCanonical(fd, &oldtio) == -1)
      return -1;

    close(fd);
    return 0;
}
