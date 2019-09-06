#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#define OUT stdout
/*
REQUIREMENTS
[X] Trunk road NS must start
[X] Green is for X seconds
[X] two seconds between red + green
[X] traffic light cycle continues 
    until there are no more vehicles to serve
[X] Vehicles comply with directions
[X] 3 controllers
[X] arrival rate

*/
//mini_controller and vehicle structs
//you may make changes if necessary
//ucpu0.ug.cs.usyd.edu.au
typedef struct mini_controller_object
{
    int id;
    int time_green;
	int min_interval;
} mini_cntrl_t;

typedef struct vehicle_object
{
    int id;
    char direction[4];
} vehicle_t;


void * mini_controller_routine(void *);
void * vehicle_routine(void *);
int dir_to_int(char[4]);
//declare global mutex and condition variables
pthread_mutex_t is_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t vehicle_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t dir_conds[6];
pthread_cond_t cont_conds[3];
char d_str[6][4] = {"n2s", "s2n", "e2w", "w2e", "n2w", "s2e"}; 
int vehicle_count[6] = {0,0,0,0,0,0};
FILE *fp;

int main(int argc, char ** argv)
{
    int n_vehicles; //total number of vehicles
	int vehicle_rate; //vehicle arrival rate to the intersection
	int min_interval; // min time interval between two consecutive vehicles to pass the intersection
    int i, rc, g_t;
    fp = fopen("logs.txt", "w+");
    if(fp == NULL){
        fprintf(stderr, "Unable to open file.\n");
    }

	//allocate memory for mini_controllers 
	pthread_t *mini_controller_thrd_id = malloc(3 * sizeof(pthread_t)); //system thread ids
    if (mini_controller_thrd_id == NULL)
    {
        fprintf(stderr, "mini_controlle_thrds_id out of memory\n");
        exit(1);
    }
    mini_cntrl_t *mini_controller = malloc(3 * sizeof(mini_cntrl_t)); //mini_controller objects
    if (mini_controller == NULL)
    {
        fprintf(stderr, "mini_controller out of memory\n");
        exit(1);
    }
	// input parameters
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
	

	//allocate memory for vehicles 
    vehicle_t* vehicle = calloc(n_vehicles, sizeof(vehicle_t));
    pthread_t* vehicle_tid = calloc(n_vehicles, sizeof(pthread_t));
    for(i=0;i<3;++i){
        rc = pthread_cond_init(&cont_conds[i], NULL);
    } 
    for(i=0;i<6;++i){
        pthread_cond_init(&dir_conds[i], NULL);
    }
	int v_count[6] = {0,0,0,0,0,0};
    //create mini_controller threads 
    for(i=0;i<3;++i){
        mini_controller[i].id = i;
        mini_controller[i].min_interval = min_interval;
        rc = pthread_create(&mini_controller_thrd_id[i], NULL, mini_controller_routine, (void *)&mini_controller[i]);
		if (rc) {
			fprintf(OUT, "ERROR; return code from pthread_create() (consumer) is %d\n", rc);
			exit(-1);
		}    
    }
	//create vehicles threads

	srand(time(0));
    for (i = 0; i<n_vehicles; i++)
    {
        //condition to ensure that if there is only 1 car left, the threads join immediately after.
		i > 0 ? sleep(vehicle_rate) : 1; 
		
        int d = (int)rand()% 6;
        strncpy(vehicle[i].direction, d_str[d], 4);
        vehicle[i].id = v_count[d];
        ++v_count[d];
		rc = pthread_create(&vehicle_tid[i], NULL, vehicle_routine, (void *)&vehicle[i]);
		if (rc) {
			fprintf(OUT, "ERROR; return code from pthread_create() (consumer) is %d\n", rc);
			exit(-1);
		}
    }
    
	//join and terminating threads.
    for(i=0;i<n_vehicles;++i)
    {
        void* rv; 
        pthread_join(vehicle_tid[i], &rv);
        vehicle_t* v = (vehicle_t*)rv;
        pthread_mutex_lock(&counter_mutex);
        --vehicle_count[dir_to_int(v->direction)];
        pthread_mutex_unlock(&counter_mutex);
    }
    //Cancel the threads once all vehicles have crossed the intersection (joined).
    for(i=0;i<3;++i)
    {
        pthread_cancel(mini_controller_thrd_id[i]);
    }

    //deallocate allocated memory
    free(mini_controller_thrd_id);
    free(mini_controller);
    free(vehicle);
    free(vehicle_tid);
	
    //destroy mutex and condition variable objects
    pthread_mutex_destroy(&vehicle_mutex);
    pthread_mutex_destroy(&is_mutex);
    pthread_mutex_destroy(&counter_mutex);
    
    for(i=0;i<3;++i){
        rc = pthread_cond_destroy(&cont_conds[i]);
    }
    for(i=0;i<6;++i){
        pthread_cond_destroy(&dir_conds[i]);
    }
    fclose(fp);
    exit(0);
}
int skip = 0;
int ready_thread[3] = {0,0,0};
struct timespec wt;
struct timeval now;
void * mini_controller_routine(void * arg)
{   
    mini_cntrl_t* c = (mini_cntrl_t*)arg;
    int dir_idx[2] = {c->id*2, c->id*2+1};
    char* dir[2] = {d_str[dir_idx[0]], d_str[dir_idx[1]]};
    int i = c->id;
    fprintf(OUT, "Traffic light %d mini-controller %s %s: Initialization complete. I am ready.\n", \
        c->id, dir[0], dir[1]);
    //Each thread locks and waits except for the trunk road.
    pthread_mutex_lock(&is_mutex);
    //If the trunk road unlocks the mutex and the other road hasn't made
    //it to the wait, skip it so it doesn't get stuck on the wait and create race cond
    ready_thread[i] = 1;
    if(i!=0){
        pthread_cond_signal(&cont_conds[0]);
        pthread_cond_wait(&cont_conds[i], &is_mutex);
    } else {
        while(ready_thread[1] != 1 || ready_thread[1] != 1){
            gettimeofday(&now, NULL);
            wt.tv_sec = now.tv_sec+10;
            wt.tv_nsec = now.tv_sec;
            pthread_cond_wait(&cont_conds[i], &is_mutex);
        }
    }
    //This thread will be cancelled when all vehicles have passed through the intersection.
    while(1){
        fprintf(OUT, "\nThe traffic lights %s %s have changed to green.\n", \
            dir[0], dir[1]);
        // Signal vehicles
        time_t begin = time(NULL);
        int remaining;
        remaining = c->time_green - difftime(time(NULL), begin);
        do{
            //signal both directions
            pthread_cond_signal(&dir_conds[dir_idx[0]]);
            pthread_cond_signal(&dir_conds[dir_idx[1]]);
            //in the case that the min interval exceed the remaining time
            //we want to avoid it waiting for too long.
            if(c->min_interval > remaining){
                sleep(remaining);
            } else {
                sleep(c->min_interval);
            }
            remaining = c->time_green - difftime(time(NULL), begin);
        }while(remaining > 0);

        fprintf(OUT, "The traffic lights %s %s will change to red now.\n\n",\
            dir[0], dir[1]);
        sleep(2);
        //signal the next condition variable then wait for it's turn.
        pthread_cond_signal(&cont_conds[(i+1)%3]);
        pthread_cond_wait(&cont_conds[i], &is_mutex);
    }
    return (void*)c;
}
//helper function to convert the direction string to an index
int dir_to_int(char c[4]){
    int i, found=-1;
    for(i = 0;i < 6;++i){
        if(strncmp(d_str[i], c, 4) == 0){
            found=1;
            break;
        }
    }
    if(found==-1){
        return -1;
    }
    return i;
}

void * vehicle_routine(void * arg)
{
    vehicle_t* v = (vehicle_t*)arg;
    int dir_idx = dir_to_int(v->direction);
    //Create a vehicle counter for debugging. Can be used if there is a lock
    //or to ensure all threads joined.
    pthread_mutex_lock(&counter_mutex);
    ++vehicle_count[dir_idx];
    pthread_mutex_unlock(&counter_mutex);
    fprintf(OUT, "Vehicle %d %s has arrived at the intersection\n", \
        v->id, v->direction);
    //must lock before waiting
    pthread_mutex_lock(&vehicle_mutex);
    pthread_cond_wait(&(dir_conds[dir_idx]), &vehicle_mutex);
    fprintf(OUT, "Vehicle %d %s is proceeding through the intersection.\n", v->id, v->direction);
    //unlock the mutex to allow another vehicle to acquire.
    pthread_mutex_unlock(&vehicle_mutex);
    return (void*)v;
}

