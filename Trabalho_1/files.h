
#pragma once

#include <stdio.h>

int getFileSize(FILE *fp);

FILE* openFile(char* fileName, char* mode);

int closeFile(FILE* fp);
