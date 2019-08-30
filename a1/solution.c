#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "logger.h"

/*
ctl is short for mini controller
dn is short for direction, dns is short for directions

*/

typedef struct controller_object{
    int id;
    int time_green;
	int min_interval;
    pthread_t tid;
} ctl_t;

typedef struct vehicle_object{
    int id;
    int dn; //direction is an int. it can be used to reference the char.
} vehicle_t;

typedef struct intersection_data{
    int n_vehicles; //total number of vehicles
    int vehicle_rate; //vehicle arrival rate to the intersection
    int min_interval; // min time interval between two consecutive vehicles to pass the intersection
    int trunk_ivl; // the amount of time the trunk road light stays green
    int minor_ivl; // the amount of time the minor road light stays green
    int turn_ivl; // the amount of time the right rturn road light stays green
}is_t;

//intersection specific helpers
enum direction{N2S, S2N, E2W, W2E, N2W, S2E}dn;

char dns[6][4] = {"n2s", "s2n", "e2w", "w2e", "n2w", "s2e"};
int v_counter[6] = {0,0,0,0,0,0}; //counts the number of vehicles created
int vt_counter[6] = {0,0,0,0,0,0}; //counts the number of vehicle threads initialised

pthread_mutex_t intersection_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex to ensure only one controller has the intersection at a time
// pthread_mutex_t vehicle_mutex_d1 = PTHREAD_MUTEX_INITIALIZER;
// pthread_mutex_t vehicle_mutex_d2 = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex so threads can increase counter atomically

pthread_cond_t ns_ctl, ew_ctl, turn_ctl;
// pthread_cond_t ns_v_cond, ew_v_cond, turn_v_cond;

void * controller_routine(void *);
void * vehicle_routine(void *);
void init_intersection(is_t*, char**);
void init_controllers(ctl_t*, is_t*);
void init_vehicles(vehicle_t*, is_t*);

int main(int argc, char ** argv){
    is_t *is_data = calloc(1, sizeof(is_data)); //core intersection data
    ctl_t *controller = calloc(3, sizeof(ctl_t)); //An array of controllers
    vehicle_t *vehicle = calloc(is_data->n_vehicles, sizeof(vehicle_t)); //An array of vehicles.
    pthread_t *vehicle_tids = calloc(is_data->n_vehicles, sizeof(pthread_t));
    pthread_t *ctl_tids = calloc(3, sizeof(pthread_t));
    int i; //response code, indexer and current dn
    
    //Initialise all necessary stuff for intersection.
    init_intersection(is_data, argc == 7 ? argv : NULL);
    init_controllers(controller, is_data);
    init_vehicles(vehicle, is_data);

    //Do a check to make sure things were initialised properly.
    if(controller == NULL || vehicle == NULL || ctl_tids == NULL || vehicle_tids == NULL){
        logger("ERROR", "Iniitalisation failed");
        exit(1);
    }
    //create threads for controllers
    for(i = 0; i< 3; ++i){
        int rc = pthread_create(&ctl_tids[i], NULL, controller_routine, (void*)&controller[i]);
            if(rc) {
                logger("ERROR", "controller thread was not created");
                exit(2);
            }
    }
    // Create threads for vehicles.
    for(i = 0; i < is_data->n_vehicles; ++i){
        int rc = pthread_create(&vehicle_tids[i], NULL, vehicle_routine, (void *)&vehicle[i]);
        if (rc) {
            logger("ERROR", "Unable to create vehicle thread");
            exit(2);
        }
    }
    //Join the vehicle threads.
    for(i = 0; i < is_data->n_vehicles; ++i){
        pthread_join(vehicle_tids[i], NULL);
    }
    for(i=0;i<3;++i){
        pthread_join(ctl_tids[i], NULL);
    }

    //Run checksums on the vehicle 
    int sumT = 0;
    for(i=0;i<6;++i){
        printf("%s : %d\n", dns[i], v_counter[i]);
        sumT += v_counter[i];
    }
    printf("Total returning are: %d\n", sumT);
	
    //destroy mutex and condition variable objects 
    pthread_mutex_destroy(&intersection_mutex);
    pthread_mutex_destroy(&counter_mutex);
    pthread_cond_destroy(&ns_ctl);
    pthread_cond_destroy(&ew_ctl);
    pthread_cond_destroy(&turn_ctl);

    free(is_data);
    free(vehicle);
    free(controller);
    free(ctl_tids);
    free(vehicle_tids);

    exit(0);
}

void * controller_routine(void * arg) {
    ctl_t * c = (ctl_t*)arg;
    printf("Mini controller %d exists!\n", c->id);
    int idx = 0;
    while(idx < 10){
        switch(c->id){
            case(0):
                pthread_mutex_lock(&intersection_mutex);
                sleep(c->time_green);
                printf("Cars are currently crossing the NS intersection\n");
                pthread_cond_signal(&ew_ctl);
                pthread_mutex_unlock(&intersection_mutex);
                pthread_cond_wait(&ns_ctl, &intersection_mutex);
                break;
            
            case(1):
                pthread_cond_wait(&ew_ctl, &intersection_mutex);
                printf("Cars are currently crossing the EW intersection\n");
                sleep(c->time_green);
                pthread_cond_signal(&turn_ctl);
                pthread_mutex_unlock(&intersection_mutex);
                break;

            case(2):
                pthread_cond_wait(&turn_ctl, &intersection_mutex);
                printf("Cars are currently turning across NW and SE\n");
                sleep(c->time_green);
                pthread_cond_signal(&ns_ctl);
                pthread_mutex_unlock(&intersection_mutex);
                break;
            default:
                fprintf(stderr, "mini controller routine is out of range\n");
                break;
        }
        ++idx;
    }
    return (void*)c;
}

void * vehicle_routine(void * arg){
    vehicle_t *v = (vehicle_t*)arg;
    pthread_mutex_lock(&counter_mutex);
    ++vt_counter[v->id];
    pthread_mutex_unlock(&counter_mutex);
    return (void*)v;
}



void init_intersection(is_t* is_data, char** argv){
    if(is_data == NULL){
        logger("ERROR", "is_data memory not able to be allocated");
        exit(2);
    }
    int g_t;
    //*****MAKE SURE I REMOVE THIS BEFORE SUBMITTING!*******//
    if(argv == NULL){
        // ask for the total number of vehicles.
        printf("Enter the total number of vehicles (int): ");
        scanf("%d", &is_data->n_vehicles);
        //ask for vehicles' arrival rate to the intersection
        printf("Enter vehicles arrival rate (int): ");
        scanf("%d", &is_data->vehicle_rate);
        //ask for the minimum time interval t (in seconds) between any two 
        //consecutive vehicles in one dn to pass through the intersection
        printf("Enter minimum interval between two consecutive vehicles (int): ");
        scanf("%d", &is_data->min_interval);
        //ask for green time for each controller
        printf("Enter green time for forward-moving vehicles on trunk road (int): ");
        scanf("%d", &g_t);
        is_data->trunk_ivl = g_t;
        printf("Enter green time for vehicles on minor road (int): ");
        scanf("%d", &g_t);
        is_data->minor_ivl = g_t;	
        printf("Enter green time for right-turning vehicles on trunk road (int): ");
        scanf("%d", &g_t);
        is_data->turn_ivl = g_t;
     } else {
        is_data->n_vehicles = atoi(argv[1]);
        is_data->vehicle_rate = atoi(argv[2]);
        is_data->min_interval = atoi(argv[3]);
        is_data->trunk_ivl = atoi(argv[4]);
        is_data->minor_ivl = atoi(argv[5]);
        is_data->turn_ivl = atoi(argv[6]);
    }
    printf("Intersection data %d %d %d %d %d %d", \
        is_data->n_vehicles, is_data->vehicle_rate, is_data->min_interval, \
        is_data->trunk_ivl, is_data->minor_ivl, is_data->turn_ivl);
}

void init_controllers(ctl_t* controller, is_t *is_data){
    int rc1, rc2, rc3, i=0;
    rc1 = pthread_cond_init(&ns_ctl, NULL);
    rc2 = pthread_cond_init(&ew_ctl, NULL);
    rc3 = pthread_cond_init(&turn_ctl, NULL);
    if(rc1 || rc2 || rc3 || controller == NULL){
        logger("ERROR", "controller condition threads didn't initialise\n");
        exit(1);
	}
    logger("INFO", "Controllers initialized");

    for(i = 0; i < 3; ++i){
        controller[i].id = i;
        controller[i].min_interval = is_data->min_interval;
    }
    return;
}

void init_vehicles(vehicle_t* vehicle, is_t* is_data){
    if(vehicle == NULL || is_data == NULL){
        logger("ERROR", "data passed to init_vehicles is incorrect");
        exit(2);
    }    
    srand(time(0));
    for(int i = 0; i < is_data->n_vehicles; ++i){
		sleep((int)rand()%is_data->vehicle_rate); 
		int dn = (int)rand()% 6; 
        vehicle[i].dn = dn;
        vehicle[i].id = v_counter[dn];
        ++v_counter[dn];
        logger("INFO", "VEHICLE initialised");
    }	
}