#include "alarm.h"

/**
 * Handles the alarm signal
 * @param signal Signal that is received
 */
void alarmHandler(int signal) {

  if(num_retr < ll.numTransmissions){
    resendFrame = true;
    printf("Timeout/invalid value: Sent frame again (numretries = %d)\n", num_retr);
    alarm(ll.timeout);
    num_retr++;

  }
  else{
    printf("Number of retries exceeded (numretries = %d)\n", num_retr);
    finish = 1;
  }
}
