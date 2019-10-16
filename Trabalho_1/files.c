
#include <stdio.h>
#include <stdlib.h>
#include "files.h"
#include "macros.h"

int getFileSize(FILE *fp){

    int lsize;
    
    fseek(fp, 0, SEEK_END);
    lsize = (int)ftell(fp);
    rewind(fp);

    return lsize;
}

FILE* openFile(char* fileName, char* mode){


    FILE* fp;
    fp = fopen(fileName, mode);
    
    if (fp == NULL)
    {
        perror(fileName);
        return NULL;
    }
    
    return fp;
}

int closeFile(FILE* fp){
    return fclose(fp);
}
