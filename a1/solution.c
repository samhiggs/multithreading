#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define OUT stdout

//mini_controller and vehicle structs
//you may make changes if necessary
//ucp0.ug.it.usyd.edu.au
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

pthread_cond_t is_cond0;
pthread_cond_t is_cond1;
pthread_cond_t is_cond2;
pthread_cond_t n2s, s2n, e2w, w2e, n2w, s2e;
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

    pthread_cond_init(&is_cond0, NULL);
    pthread_cond_init(&is_cond1, NULL);
    pthread_cond_init(&is_cond2, NULL);

    cont_conds[0] = is_cond0;
    cont_conds[1] = is_cond1;
    cont_conds[2] = is_cond2;

    pthread_cond_init(&n2s, NULL);
    pthread_cond_init(&s2n, NULL);
    pthread_cond_init(&e2w, NULL);
    pthread_cond_init(&w2e, NULL);
    pthread_cond_init(&n2w, NULL);
    pthread_cond_init(&s2e, NULL);

    dir_conds[0] = n2s; 
    dir_conds[1] = s2n; 
    dir_conds[2] = e2w; 
    dir_conds[3] = w2e; 
    dir_conds[4] = n2w; 
    dir_conds[5] = s2e; 


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
	int v_count[6] = {0,0,0,0,0,0};
    //create mini_controller threads 
    for(i=0;i<3;++i){
        mini_controller[i].id = i;
        mini_controller[i].min_interval = min_interval;
        rc = pthread_create(&mini_controller_thrd_id[i], NULL, mini_controller_routine, (void *)&mini_controller[i]);
		if (rc) {
			fprintf(OUT, "ERROR; return code from pthread_create() (consumer) is %d\n", rc);
			exit(-1);
		}    }
	//create vehicles threads
	srand(time(0));
    for (i = 0; i<n_vehicles; i++)
    {
		int d = (int)rand()% 6;
        strncpy(vehicle[i].direction, d_str[d], 4);
        vehicle[i].id = v_count[d];
        ++v_count[d];
		rc = pthread_create(&vehicle_tid[i], NULL, vehicle_routine, (void *)&vehicle[i]);
		if (rc) {
			fprintf(OUT, "ERROR; return code from pthread_create() (consumer) is %d\n", rc);
			exit(-1);
		}
		sleep(vehicle_rate); 
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
    fprintf(OUT, "Vehicles Joined: %d %d %d %d %d %d\n", \
    vehicle_count[0], vehicle_count[1], vehicle_count[2], \
    vehicle_count[3], vehicle_count[4], vehicle_count[5]);
    for(i=0;i<3;++i)
    {
        pthread_join(mini_controller_thrd_id[i], NULL);
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
    
    pthread_cond_destroy(&is_cond0);
    pthread_cond_destroy(&is_cond1);
    pthread_cond_destroy(&is_cond2);

    pthread_cond_destroy(&n2s);
    pthread_cond_destroy(&s2n);
    pthread_cond_destroy(&e2w);
    pthread_cond_destroy(&w2e);
    pthread_cond_destroy(&n2w);
    pthread_cond_destroy(&s2e);

    fclose(fp);
	
    exit(0);
}

void * mini_controller_routine(void * arg)
{   
    mini_cntrl_t* c = (mini_cntrl_t*)arg;
    int dir_idx[2] = {c->id*2, c->id*2+1};
    char* dir[2] = {d_str[dir_idx[0]], d_str[dir_idx[1]]};
    fprintf(OUT, "Traffic light %d mini-controller %s %s: Initialization complete. I am ready.\n", \
        c->id, dir[0], dir[1]);
    if(c->id==0){
        pthread_mutex_lock(&is_mutex);
    } else if(c->id == 1){
        pthread_cond_wait(&is_cond1, &is_mutex);
    } else {
        pthread_cond_wait(&is_cond2, &is_mutex);
    }
        // pthread_mutex_lock(&is_mutex);
    while(1){
        fprintf(OUT, "The traffic lights %s %s have changed to green.\n", \
            dir[0], dir[1]);
        // Signal vehicles
        clock_t begin = clock();
        int curr;
        do{
            // fprintf(OUT, "signalling\n");
            pthread_cond_signal(&dir_conds[dir_idx[0]]);
            pthread_cond_signal(&dir_conds[dir_idx[1]]);
            sleep(c->min_interval);
            curr = (int)(clock()-begin)/CLOCKS_PER_SEC;
        }while(curr < c->time_green);

        fprintf(OUT, "The traffic lights %s %s will change to red now.\n\n",\
            dir[0], dir[1]);
        
        if(c->id==0){
            pthread_cond_signal(&is_cond1);
            pthread_cond_wait(&is_cond0, &is_mutex);
        }
        if(c->id==1){
            pthread_cond_signal(&is_cond2);
            pthread_cond_wait(&is_cond1, &is_mutex);
        }
        if(c->id==2){
            pthread_cond_signal(&is_cond0);
            pthread_cond_wait(&is_cond2, &is_mutex);
        }
        // pthread_mutex_unlock(&is_mutex);
    }


    return (void*)c;
}
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
    pthread_mutex_lock(&counter_mutex);
    ++vehicle_count[dir_idx];
    pthread_mutex_unlock(&counter_mutex);
    if(dir_idx == -1){
        fprintf(stderr, "The diretion doesn't exist\n");
        exit(-1);
    }
    fprintf(OUT, "Vehicle %d %s %d has arrived at the intersection\n", \
        v->id, v->direction, dir_idx);
    pthread_mutex_lock(&vehicle_mutex);
    pthread_cond_wait(&(dir_conds[dir_idx]), &vehicle_mutex);
    fprintf(OUT, "Vehicle %d %s is proceeding through the intersection.\n", v->id, v->direction);
    pthread_mutex_unlock(&vehicle_mutex);
    return (void*)v;
}

