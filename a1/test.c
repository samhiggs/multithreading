#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stdout, "Please input filename to parse.\n");
        exit(-1);
    }
    char str[200];
    FILE* fp;
    char* fn = argv[1];
    fp = fopen(fn, "r");
    if(fp == NULL){
        fprintf(stdout, "Unable to read filename. Please check\n %s\n", \
        fn);
        exit(-1);
    }
    while(1) {
      while (fgets(str, 200, fp) != NULL)
        printf("%s", str);
   }
    fclose(fp);
}