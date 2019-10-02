#pragma once

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "data_link.h"
#include "macros.h"
#include "app.h"
#include "aux.h"

int finish, num_retr;

void alarmHandler(int signal);
