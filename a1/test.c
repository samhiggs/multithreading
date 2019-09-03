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

void does_stuff(vehicle_t** v){
    v = calloc(3, sizeof(vehicle_t));
    v[0]->id = 2;
    v[0]->dn = 20;
}

int main(int argc, char** argv){
    vehicle_t** v = NULL;
    does_stuff(v);
    printf("SHOULD GET HERE\n");
    printf("%d %d\n", v[0]->id, v[0]->dn);
    return 0;
}
