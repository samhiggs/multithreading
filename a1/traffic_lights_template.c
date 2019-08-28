#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

//mini_controller and vehicle structs
//you may make changes if necessary

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

//declare global mutex and condition variables

... // you need to add something here



int main(int argc, char ** argv)
{
    int n_vehicles; //total number of vehicles
	int vehicle_rate; //vehicle arrival rate to the intersection
	int min_interval; // min time interval between two consecutive vehicles to pass the intersection
	
	//more variable declarations
	
	... // you need to add something here

	
	
    // Initialize mutex and condition variables
 
	... // you need to add something here if necessary
	

	
	//allocate memory for mini_controllers 
	mini_controller_thrd_id = malloc(3 * sizeof(pthread_t)); //system thread ids
    if (mini_controller_thrd_id == NULL)
    {
        fprintf(stderr, "mini_controlle_thrds_id out of memory\n");
        exit(1);
    }
    mini_controller = malloc(3 * sizeof(mini_cntrl_t)); //mini_controller objects
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
	
	... // you need to add something here
	

	
    //create mini_controller threads 
	
	... // you need to add something here
	
	
	
	//create vehicles threads

	... // you need to add something here
	
	
    
	//join and terminating threads.

	... // you need to add something here
	
	

    //deallocate allocated memory
	
	... // you need to add something here
	

	
    //destroy mutex and condition variable objects
 
	... // you need to add something here
	
	
	
    exit(0);
}

void * mini_controller_routine(void * arg)
{


... // you need to implement mini_controller routine


}

void * vehicle_routine(void * arg)
{


... // you need to implement vehicle routine


}

