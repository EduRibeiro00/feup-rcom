
#include <stdio.h>
#include <stdlib.h>
//#include "files.h"
int file_read(char* file_name)
{

    FILE *fp;
    char data[512];
    int length_read;

    fp = fopen(file_name, "r");
    if (fp == NULL)
    {
        perror(file_name);
        return -1;
    }

    while (1)
    {

        length_read = fread(data, 1, 512, fp);
        if (length_read != 512)
        {
            if (feof(fp))
            {
                printf("sucesso\n");
                //manda a ultima trama de dados
                return 0;
            }
            else
            {
                perror("error reading file data");
                return -1;
            }
        }

        //preparar a trama de dados e passar para o llwrite
    }

    fclose(fp);
    return 0;
}


int file_writer(FILE *fp, char* data){

   int length_wrote = fwrite(data, 1, 512, fp);
   if(length_wrote != 512){
            if(feof(fp)){
                
                printf("Sucesso");

                //Manda a ultima trama
                return 0;
            }
            else{
                perror("error reading file data");
                return -1;
            }

        }

    fclose(fp);
    return 0;
}