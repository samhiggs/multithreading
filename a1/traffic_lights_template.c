#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

//mini_controller and vehicle structs
//you may make changes if necessary

typedef struct mini_controller_object{
    int id;
    int time_green;
	int min_interval;
} mini_cntrl_t;

typedef struct vehicle_object{
    int id;
    char direction[4];
} vehicle_t;

enum direction{N2S, S2N, E2W, W2E, N2W, S2E};

void * mini_controller_routine(void *);
void * vehicle_routine(void *);

pthread_mutex_t intersection_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ns_mini_cntrl, ew_mini_cntrl, turn_mini_cntrl;
vehicle_t *vehicle;
int *v_counter;
char directions[6][4] = {"n2s", "s2n", "e2w", "w2e", "n2w", "s2e"};
int main(int argc, char ** argv)
{
    int n_vehicles; //total number of vehicles
	int vehicle_rate; //vehicle arrival rate to the intersection
	int min_interval; // min time interval between two consecutive vehicles to pass the intersection
    int rc, i, drct;
    int g_t;
    int ns_c, sn_c, ew_c, we_c, nw_c, se_c;
    int n_cntrls = 3; //number of mini controllers.
    pthread_t *vehicle_thread_ids, *mini_cntrl_thread_ids;
    mini_cntrl_t *mini_controller;

    rc = pthread_cond_init(&ns_mini_cntrl, NULL);
    if(rc){
        fprintf(stderr, "ERROR; return code from pthread_cond_init(ns) is %d\n", rc);
        exit(1);
	}
    rc = pthread_cond_init(&ew_mini_cntrl, NULL);
    if(rc){
        fprintf(stderr, "ERROR; return code from pthread_cond_init(ew) is %d\n", rc);
        exit(1);
	}
    rc = pthread_cond_init(&turn_mini_cntrl, NULL);
    if(rc){
        fprintf(stderr, "ERROR; return code from pthread_cond_init(turn)  is %d\n", rc);
        exit(1);
	}
	//allocate memory for mini_controllers 
	mini_cntrl_thread_ids = calloc(3, sizeof(pthread_t)); //system thread ids
    if (mini_cntrl_thread_ids == NULL)
    { 
        fprintf(stderr, "mini_cntrl_thread_ids out of memory\n");
        exit(2);
    }
    mini_controller = calloc(3, sizeof(mini_cntrl_t)); //mini_controller objects
    if (mini_controller == NULL)
    { 
        fprintf(stderr, "mini controller out of memory\n");
        exit(2);
    }
    //*****MAKE SURE I REMOVE THIS BEFORE SUBMITTING!*******//
    if(argc != 5){
        // ask for the total number of vehicles.
        printf("Enter the total number of vehicles (int): ");
        scanf("%d", &n_vehicles);
        //ask for vehicles' arrival rate to the intersection
        printf("Enter vehicles arrival rate (int): \n");
        scanf("%d", &vehicle_rate);
        //ask for the minimum time interval t (in seconds) between any two 
        //consecutive vehicles in one direction to pass through the intersection
        printf("Enter minimum interval between two consecutive vehicles (int): \n");
        scanf("%d", &min_interval);
        //ask for green time for each mini_controller
        printf("Enter green time for forward-moving vehicles on trunk road (int): ");
        scanf("%d", &g_t);
        mini_controller[0].time_green = g_t;
        printf("Enter green time for vehicles on minor road (int): ");
        scanf("%d", &g_t);
        mini_controller[1].time_green = g_t;	
        printf("Enter green time for right-turning vehicles on trunk road (int): ");
        scanf("%d", &g_t);
        mini_controller[2].time_green = g_t;
    } else {
        n_vehicles = atoi(argv[1]);
        vehicle_rate = atoi(argv[2]);
        min_interval = atoi(argv[3]);
        int green_time = atoi(argv[4]);
        mini_controller[0].time_green = green_time;
        mini_controller[1].time_green = green_time;
        mini_controller[2].time_green = green_time;
    }

    //Start a vehicle counter in each direction to make sure they all finish 
    //or if not, where it gets blocked.
	v_counter = calloc(6, sizeof(int));
	//create mini_controller threads 
    if (mini_controller == NULL){
        fprintf(stderr, "mini_controller out of memory\n");
        exit(2);
    }
    for(i = 0; i < n_cntrls; ++i)
    {
        mini_controller[i].id = i;
        mini_controller[i].min_interval = min_interval;
        rc = pthread_create(&mini_cntrl_thread_ids[i], NULL, mini_controller_routine, (void*)&mini_controller[i]);
        if (rc) 
        {
            fprintf(stderr, "return code from pthread_create(ns controller) is %d\n", rc);
            exit(3);
        }
    }

    //allocate memory for vehicles and threads
    vehicle = calloc(n_vehicles, sizeof(vehicle_t));
    vehicle_thread_ids = calloc(n_vehicles, sizeof(pthread_t)); //mini_controller objects
    if (vehicle_thread_ids == NULL)
    { 
        fprintf(stderr, "vehicle thread out of memory\n");
        exit(2);
    }
	//create vehicles threads
    ns_c = sn_c = ew_c = we_c = nw_c = se_c = 0;
    // srand(time(0));
    for(i = 0; i < n_vehicles; ++i){
		sleep((int)rand() % vehicle_rate); 
		drct = (int)rand()% 6;
        switch(drct)
        {
            case N2S: 
                strncpy(vehicle[i].direction, "n2s", 4);
                vehicle[i].id = ns_c;
                ++ns_c;
                break;
            case S2N: 
                strncpy(vehicle[i].direction, "s2n", 4);
                vehicle[i].id = sn_c;
                ++sn_c;
                break;
            case E2W: 
                strncpy(vehicle[i].direction, "e2w", 4);
                vehicle[i].id = ew_c;
                ++ew_c;
                break;
            case W2E: 
                strncpy(vehicle[i].direction, "w2e", 4);
                vehicle[i].id = we_c;
                ++we_c;
                break;
            case N2W: 
                strncpy(vehicle[i].direction, "n2w", 4);
                vehicle[i].id = nw_c;
                ++nw_c;
                break;
            case S2E: 
                strncpy(vehicle[i].direction, "s2e", 4);
                vehicle[i].id = se_c;
                ++se_c;
                break;
            default:
                fprintf(stderr, "Vehicle doesn't have direction %d\n", drct);
                exit(7);
        }
		rc = pthread_create(&vehicle_thread_ids[i], NULL, vehicle_routine, (void *)&vehicle[i]);
		if (rc) {
			printf("ERROR; return code from pthread_create(vehicle) is %d\n", rc);
			exit(4);
		}
    }	
	//join and terminating threads.
    //count the cars that join
    for(i = 0; i < n_vehicles; ++i){
        pthread_join(vehicle_thread_ids[i], NULL);
    }
    int sumT = 0;
    for(i=0;i<6;++i){
        printf("%s : %d\n", directions[i], v_counter[i]);
        sumT += v_counter[i];
    }
    printf("Total returning are: %d\n", sumT);

    printf("\n");
    for(i = 0;i < n_cntrls; ++i){
        pthread_join(mini_cntrl_thread_ids[i], NULL);
    }
		
    //destroy mutex and condition variable objects
 
    pthread_mutex_destroy(&intersection_mutex);
    pthread_mutex_destroy(&counter_mutex);
    pthread_cond_destroy(&ns_mini_cntrl);
    pthread_cond_destroy(&ew_mini_cntrl);
    pthread_cond_destroy(&turn_mini_cntrl);

    free(vehicle);
    free(mini_cntrl_thread_ids);
    free(vehicle_thread_ids);
    free(mini_controller);

    exit(0);
}

void * mini_controller_routine(void * arg) {
    mini_cntrl_t * c = (mini_cntrl_t*)arg;
    printf("Mini controller %d exists!\n", c->id);
    int idx = 0;
    while(idx < 10){
        switch(c->id){
            case(0):
                pthread_mutex_lock(&intersection_mutex);
                sleep(c->time_green);
                printf("Cars are currently crossing the NS intersection\n");
                pthread_cond_signal(&ew_mini_cntrl);
                pthread_mutex_unlock(&intersection_mutex);
                pthread_cond_wait(&ns_mini_cntrl, &intersection_mutex);
                break;
            
            case(1):
                printf("Cars are currently crossing the EW intersection\n");
                pthread_cond_wait(&ew_mini_cntrl, &intersection_mutex);
                sleep(c->time_green);
                pthread_cond_signal(&turn_mini_cntrl);
                pthread_mutex_unlock(&intersection_mutex);
                break;

            case(2):
                printf("Cars are currently turning across NW and SE\n");
                pthread_cond_wait(&turn_mini_cntrl, &intersection_mutex);
                sleep(c->time_green);
                pthread_cond_signal(&ns_mini_cntrl);
                pthread_mutex_unlock(&intersection_mutex);
                break;
            default:
                fprintf(stderr, "mini controller routine is out of range\n");
                break;
        }
        ++idx;
    }
}

void * vehicle_routine(void * arg){
    vehicle_t *v = (vehicle_t*)arg;
    pthread_mutex_lock(&counter_mutex);
    if(!strncmp(v->direction, "n2s", 4))
        ++v_counter[N2S];
    else if(!strncmp(v->direction, "s2n", 4))
        ++v_counter[S2N];
    else if(!strncmp(v->direction, "e2w", 4))
        ++v_counter[E2W];
    else if(!strncmp(v->direction, "w2e", 4))
        ++v_counter[W2E];
    else if(!strncmp(v->direction, "n2w", 4))
        ++v_counter[N2W];
    else if(!strncmp(v->direction, "s2e", 4))
        ++v_counter[S2E];
    pthread_mutex_unlock(&counter_mutex);
}

