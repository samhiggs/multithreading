#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

typedef struct child_object
{
    int id;
    int play_time;
} child_obj;

void * child_routine(void *);

//declare global mutex and condition variables
pthread_mutex_t toy = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c0_cond, c1_cond, c_test_cond;
int iter = 0;
int main(int argc, char ** argv)
{
	pthread_t *child_thrd_id; //system thread id
	child_obj *child; //user-defined thread id
	int total_time; //total amount of time childs can play
 	int k, b_t, rc;
	child_thrd_id = malloc(2 * sizeof(pthread_t)); //system thread ids
	if(child_thrd_id == NULL){
		fprintf(stderr, "threads out of memory\n");
		exit(1);
	}	
	
	child = malloc(2 * sizeof(child_obj)); //child objects
	if(child == NULL){
		fprintf(stderr, "t out of memory\n");
		exit(1);
	}	

	//Initialize condition variable objects 
	rc = pthread_cond_init(&c0_cond, NULL);
	if(rc){
		printf("ERROR; return code from pthread_cond_init() (s2n) is %d\n", rc);
		exit(-1);
	}
	rc = pthread_cond_init(&c1_cond, NULL);
	if(rc){
		printf("ERROR; return code from pthread_cond_init() (s2n) is %d\n", rc);
		exit(-1);
	}
	rc = pthread_cond_init(&c_test_cond, NULL);
	if(rc){
		printf("ERROR; return code from pthread_cond_init() (s2n) is %d\n", rc);
		exit(-1);
	}
	if(argc != 4){
			// ask for the total time.
		printf("Enter the total amount children can play (int): \n");
		scanf("%d", &total_time);
		
		//ask for children playing time 
		printf("Enter child 0 playing time (int): \n");
		scanf("%d", &b_t);
		child[0].play_time = b_t;
		printf("Enter child 1 playing time (int): \n");
		scanf("%d", &b_t);
		child[1].play_time = b_t;
	} else {
		total_time = atoi(argv[1]);
		child[0].play_time = atoi(argv[2]);
		child[1].play_time = atoi(argv[3]);
	}

	//create child threads 
    for (k = 0; k<2; k++){
		child[k].id = k;
		rc = pthread_create(&child_thrd_id[k], NULL, child_routine, (void *)&child[k]);
		if (rc) {
			printf("ERROR; return code from pthread_create() (child) is %d\n", rc);
			exit(-1);
		}
    }
    
	//sleep total_time seconds
	sleep(total_time);
	
	//Time is up and "the children's parent" calls the children to stop playing, i.e., terminate child threads.
    for (k = 0; k<2; k++) {
		pthread_cancel(child_thrd_id[k]); 
    }

	//deallocate allocated memory
	free(child_thrd_id);
	free(child);

	pthread_mutex_destroy(&toy);
	pthread_cond_destroy(&c0_cond);	
	pthread_cond_destroy(&c1_cond);	
	pthread_cond_destroy(&c_test_cond);	
    exit(0);
}
// 0 is false, 1 is true
void * child_routine(void * arg){
	child_obj* child = (child_obj*)arg;
	while(1){
		pthread_mutex_lock(&toy);
		printf("iteration : %d\nchild %d: I get to play with the toy for %d units of time.\n", ++iter, child->id, child->play_time);
		sleep(child->play_time);
		printf("child %d: I now give the toy to child %d\n\n", child->id, (child->id+1)%2);
		pthread_cond_signal(&c_test_cond);
		pthread_mutex_unlock(&toy);
		pthread_cond_wait(&c_test_cond, &toy);
	}
	
}
