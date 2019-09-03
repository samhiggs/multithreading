#include "solution.h"


/*
ctl is short for mini controller
dn is short for direction, dns is short for directions

*/

pthread_mutex_t intersection_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex to ensure only one controller has the intersection at a time
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex so threads can increase counter atomically
pthread_mutex_t vehicle_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t *cond_ctls; //contains the three condition controls in order, trunk, minor, turn
pthread_cond_t *cond_vehicles; //contains six condition controls in order: trunk, minor, turn


int main(int argc, char ** argv){
    
    int i; //response code, indexer and current dn
    
    is_t *is_data = calloc(1, sizeof(is_data)); //core intersection data
    ctl_t *controller = calloc(N_CONTROLLERS, sizeof(ctl_t)); //An array of controllers
    
    vehicle_t *vehicle = calloc(is_data->n_vehicles, sizeof(vehicle_t)); //An array of vehicles.
    pthread_t *vehicle_tids = calloc(is_data->n_vehicles, sizeof(pthread_t));

    cond_vehicles = calloc(N_DIRECTIONS, sizeof(pthread_cond_t));
    pthread_t *ctl_tids = calloc(N_CONTROLLERS, sizeof(pthread_t));
    cond_ctls = calloc(N_CONTROLLERS, sizeof(pthread_cond_t));
    
    
    //Initialise all necessary stuff for intersection.
    init_intersection(is_data, argc == 7 ? argv : NULL);
   
    init_controllers(controller, is_data);
   
    init_vehicles(vehicle, vehicle_tids, is_data);

    //Do a check to make sure things were initialised properly.
    if(controller == NULL || vehicle == NULL || ctl_tids == NULL || vehicle_tids == NULL){
        logger("ERROR", "Iniitalisation failed");
        exit(1);
    }

    //Create mini-controller threads
    for(i = 0; i< N_CONTROLLERS; ++i){
        int rc = pthread_create(&ctl_tids[i], NULL, controller_routine, (void*)&controller[i]);
            if(rc) {
                logger("ERROR", "controller thread was not created");
                exit(2);
            }
    }
    //Join mini-controller threads
    for(i=0;i<N_CONTROLLERS;++i)
        pthread_join(ctl_tids[i], NULL);
    //cancel any remaining vehicles
    for(i = 0; i < is_data->n_vehicles; ++i)
        pthread_cancel(vehicle_tids[i]);
	
    check_vehicles();
    //destroy mutex and condition variable objects 
    pthread_mutex_destroy(&intersection_mutex);
    pthread_mutex_destroy(&counter_mutex);
    for(i = 0;i < N_CONTROLLERS;++i){
        pthread_cond_destroy(cond_ctls[i]);
    }
    free(is_data);
    free(vehicle);
    free(controller);
    free(ctl_tids);
    free(vehicle_tids);

    exit(0);
}

void signal_vehicles(ctl_t* c){
    clock_t begin = clock();
    int curr;
    do{
        fprintf(stdout, "%d Signalling vehicles...\n", c->id);
        sleep(c->min_interval);
        curr = (int)(clock()-begin)/CLOCKS_PER_SEC;
    }while(curr < c->time_green);
    return;
} 
void * controller_routine(void * arg) {
    ctl_t * c = (ctl_t*)arg;
    fprintf(stdout, "Traffic light mini-controller %s %s: Initialization complete. I am ready.\n", dir_str[c->id*2], dir_str[c->id*2+1]);
    
    if(c->id == 0)
        pthread_mutex_lock(&intersection_mutex);
    else
        pthread_cond_wait(&cond_ctls[0], &intersection_mutex);
    
    while(1){
        fprintf(stdout, "The traffic lights %s %s have changed to green.\n", \
            dir_str[c->id*2], dir_str[c->id*2+1]);
        signal_vehicles(c);
        // sleep(c->time_green);
        fprintf(stdout, "The traffic lights %s %s will change to red now.\n\n",\
            dir_str[c->id*2], dir_str[c->id*2+1]);
        pthread_cond_signal(&cond_ctls[0]);
        pthread_mutex_unlock(&intersection_mutex);
        pthread_cond_wait(&cond_ctls[0], &intersection_mutex);
    }
    return (void*)c;
}
void * vehicle_routine(void * arg){
    vehicle_t *v = (vehicle_t*)arg;
    //Counter stuff for checking later.
    // fprintf(stdout, "Vehicle %d %s has arrived at the intersection\n", v->id, dns[v->dn]);
    // fprintf(stdout, "Vehicle %d %s is proceeding through the intersection.\n", v->id, dns[v->dn]);
    // pthread_mutex_lock(&vehicle_mutex);
    // pthread_cond_wait(&cond_vehicles[v->dn], &vehicle_mutex);
    // pthread_mutex_unlock(&vehicle_mutex);
    return (void*)v;
}



void init_intersection(is_t* is_data, char** argv){
    if(is_data == NULL){
        logger("ERROR", "is_data memory not able to be allocated");
        exit(2);
    }
    int g_t;
    is_data->time_green = calloc(N_CONTROLLERS, sizeof(int));
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
        is_data->time_green[0] = g_t;
        printf("Enter green time for vehicles on minor road (int): ");
        scanf("%d", &g_t);
        is_data->time_green[1] = g_t;	
        printf("Enter green time for right-turning vehicles on trunk road (int): ");
        scanf("%d", &g_t);
        is_data->time_green[2] = g_t;
     } else {
        is_data->n_vehicles = atoi(argv[1]);
        is_data->vehicle_rate = atoi(argv[2]);
        is_data->min_interval = atoi(argv[3]);
        is_data->time_green[0] = atoi(argv[4]);
        is_data->time_green[1] = atoi(argv[5]);
        is_data->time_green[2] = atoi(argv[6]);
    }
    printf("Intersection data %d %d %d %d %d %d\n", \
        is_data->n_vehicles, is_data->vehicle_rate, is_data->min_interval, \
        is_data->time_green[0], is_data->time_green[1], is_data->time_green[2]);
}

void init_controllers(ctl_t* controller, is_t *is_data){
    int rc1, rc2, i;
    for(i=0;i<N_CONTROLLERS;++i){
        rc1 = pthread_cond_init(&cond_ctls[i], NULL);
        rc2 = pthread_cond_init(&cond_vehicles[i], NULL);
        if(rc1 || rc2){
            logger("ERROR", "controller condition threads didn't initialise");
            exit(1);
        }
    }
    logger("INFO", "conditions initialized");
    for(i=0; i<is_data->n_vehicles;++i){
        controller[i].id = i;
        controller[i].min_interval = is_data->min_interval;
        controller[i].time_green = is_data->time_green[i];
        controller[i].cont_dir[0] = i*2;
        controller[i].cont_dir[1] = i*2+1;
    }
    return;
}

void init_vehicles(vehicle_t* vehicle, pthread_t* tids, is_t* is_data){
    if(vehicle == NULL || is_data == NULL){
        logger("ERROR", "data passed to init_vehicles is incorrect");
        exit(2);
    }    
    srand(time(0));
    for(int i = 0; i < is_data->n_vehicles; ++i){
		sleep((int)rand()%is_data->vehicle_rate); 
		int dn = (int)rand()% N_DIRECTIONS; 
        vehicle[i].dn = dn;
        vehicle[i].id = v_counter[dn];
        ++v_counter[dn];
        int rc = pthread_create(&tids[i], NULL, vehicle_routine, (void *)&vehicle[i]);
        if (rc) {
            logger("ERROR", "Unable to create vehicle thread");
            exit(2);
        }
        sleep(is_data->min_interval);
    }	
}

//To be completed. Meant to help with debugging.
int check_vehicles(){
    //Run checksums on the vehicle 
    int i, sumT = 0;

    for(i=0;i<N_DIRECTIONS;++i){
        printf("%s : %d\n", dir_str[i], v_counter[i]);
        sumT += v_counter[i];
    }
    printf("Total returning are: %d\n", sumT);
    return 0;
}