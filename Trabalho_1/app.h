#pragma once

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

struct applicationLayer {
    int fileDescriptor; /*Descritor correspondente à porta série*/
    int status; /*TRANSMITTER | RECEIVER*/
};