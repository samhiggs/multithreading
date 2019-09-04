#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

typedef struct vehicle_object{
    int id;
    int dn; //direction is an int. it can be used to reference the char.
} vehicle_t;

pthread_cond_t* cond1, cond2;
pthread_mutex_t* res = PTHREAD_MUTEX_INITIALIZER;

void does_stuff(vehicle_t** v){
    v = calloc(3, sizeof(vehicle_t));
    v[0]->id = 2;
    v[0]->dn = 20;
}

int main(int argc, char** argv){
    pthread_cond_init(&cond1)
    return 0;
}


void* worker(void* c){

}

void* controller(void* v){
    while(1){

    }
}