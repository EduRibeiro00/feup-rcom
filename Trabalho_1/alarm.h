#pragma once

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdbool.h>
#include "data_link.h"
#include "macros.h"
#include "app.h"
#include "aux.h"


int finish, num_retr;
bool resendFrame;

/**
 * Handles the alarm signal
 * @param signal Signal that is received
 */
void alarmHandler(int signal);
