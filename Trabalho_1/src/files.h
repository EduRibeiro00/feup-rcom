
#pragma once

#include <stdio.h>

/**
 * Auxiliary function to obtain the size of a file, from its file pointer
 * @param fp File pointer to the file
 * @return size of the file in question
 */
int getFileSize(FILE *fp);

/**
 * Function that opens a file, from its name
 * @param fileName Name of the file to be opened
 * @param mode Mode in which to open the file
 * @return file pointer to the file in question
 */
FILE* openFile(char* fileName, char* mode);

/**
 * Function that closes a file, from its file pointer
 * @param fp File pointer to the file to be closed
 * @return 0 if successful, EOF if an error occurs
 */
int closeFile(FILE* fp);
