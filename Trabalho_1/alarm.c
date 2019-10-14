#include "alarm.h"

void alarmHandler(int signal) {

  if(num_retr < ll.numTransmissions){
    sendFrame(ll.frame, al.fileDescriptor, ll.frameLength);
    printf("Timeout/invalid value: Sent frame again (numretries = %d)\n", num_retr);
    alarm(ll.timeout);
    num_retr++;

  }
  else{
    printf("Number of retries exceeded (numretries = %d)\n", num_retr);
    finish = 1;
  }
}
