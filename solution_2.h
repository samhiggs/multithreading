#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"


#define N_CONTROLLERS 3
#define N_DIRECTIONS 6

typedef struct controller_object{
    int id;
    int time_green;
	int min_interval;
    int cont_dir[2];
} ctl_t;

typedef struct vehicle_object{
    int id;
    int dn; //direction is an int. it can be used to reference the char.
} vehicle_t;

typedef struct intersection_data{
    int n_vehicles; //total number of vehicles
    int vehicle_rate; //vehicle arrival rate to the intersection
    int min_interval; // min time interval between two consecutive vehicles to pass the intersection
    int *time_green; // the amount of time the lights stay green
}is_t;

//intersection specific helpers
enum direction{N2S, S2N, E2W, W2E, N2W, S2E}dn;

char dir_str[N_DIRECTIONS][4] = {"n2s", "s2n", "e2w", "w2e", "n2w", "s2e"};
int v_counter[N_DIRECTIONS] = {0,0,0,0,0,0}; //counts the number of vehicles created
int vt_counter[N_DIRECTIONS] = {0,0,0,0,0,0}; //counts the number of vehicle threads initialised


void * controller_routine(void *);
void * vehicle_routine(void *);
void init_intersection(is_t*, char**);
void init_controllers(ctl_t*, is_t*);
void init_vehicles(vehicle_t*, pthread_t*, is_t*);
int check_vehicles();
void signal_vehicles(ctl_t*);