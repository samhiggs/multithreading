#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char** argv){
    clock_t begin, end;
    int t;
    begin = clock();
    t = (int)begin/CLOCKS_PER_SEC;
    printf("time is: %d\n", t);
    sleep(2);
    end = clock();
    t = (int)(end - begin)/CLOCKS_PER_SEC;
    printf("time is: %d\n", t);
    return 1;
 }