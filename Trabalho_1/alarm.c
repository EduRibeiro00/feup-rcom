#include "alarm.h"

void alarmHandler(int signal){

  if(num_retr < NUM_RETR){
    sendFrame(ll.frame, fd);
    printf("Timeout: Sent SET Frame again (numretries = %d)\n", num_retr);
    alarm(TIMEOUT);
    num_retr++;

  }
  else{
    printf("Number of retries exceeded (numretries = %d)\n", num_retr);
    finish = 1;
  }
}
