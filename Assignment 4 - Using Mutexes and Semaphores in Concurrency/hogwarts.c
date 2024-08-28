#define _XOPEN_SOURCE 500 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <openssl/sha.h>

#define NUM_ELVES 5

/* Please don't change anything between this line and 
   the start of the global variables */
#define HASH_BUFFER_LENGTH 32

unsigned int random_seed = 252;
int task_id_counter = 0;

typedef struct {
		int id;
		unsigned char* input;
} task;

typedef struct node {
		task* task;
		struct node* next;
} node;

node* list_head;

/* Global Variables -- Add more if you need! */
int total_tasks;
int active_tasks;
int count;
pthread_mutex_t mutex;
pthread_mutex_t canc;
sem_t empty_list;
sem_t full_list;
sem_t cancel;

/* Function Prototypes for pthreads */
void* dobby( void* );
void* house_elf( void * );

/* Don't change the function prototypes below;
   they are your API for how the house elves do work */

/* Removes a task from the list of things to do
   To be run by: house elf threads
   Takes no arguments
   NOT thread safe!
return: a pointer to a task for an elf to do
*/
task* take_task();

/* Perform the provided task
   To be run by: house elf threads
argument: pointer to task to be done
IS thread safe
return: nothing
*/  
void do_work( task* todo );

/* Put tasks in the list for elves to do
   To be run by: Dobby
argument: how many tasks to put in the list
NOT thread safe
return: nothing
*/
void post_tasks( int howmany );

/* Used to unlock a mutex, if necessary, if a thread
   is cancelled when blocked on a semaphore
   */
void house_elf_cleanup( void * );

/* Complete the implementation of main() */

int main( int argc, char** argv ) {
		if ( argc != 2 ) {
				printf( "Usage: %s total_tasks\n", argv[0] );
				return -1;
		}

		pthread_t *dob = malloc(sizeof(pthread_t) * 1);
		pthread_t *p_tids = malloc(sizeof(pthread_t) * 5);
		/* Init global variables here */
		list_head = NULL;
		count = 0;
		active_tasks = 0;
		total_tasks = atoi( argv[1] );
		pthread_mutex_init(&mutex, NULL);
		pthread_mutex_init(&canc, NULL);
		sem_init(&empty_list, 0, 0);
		sem_init(&full_list, 0, 0);
		sem_init(&cancel, 0, 0);

		pthread_mutex_lock(&canc);

		printf("There are %d tasks to do today.\n", total_tasks);

		/* Launch threads here */
		//Create 1 dobby thread
		pthread_create(&dob[0], NULL, dobby, p_tids);
		//Create 5 house elf threads
		for(int i = 0; i < NUM_ELVES; i++){
				pthread_create(&p_tids[i] , NULL, house_elf, NULL);
		}
		
		//Join dobby thread
		pthread_join(dob[0], NULL);
		//Join 5 house elves thread
		/*
		for(int i = 1; i <= NUM_ELVES; i++){
				pthread_join(p_tids[i], NULL);
		}
		*/

		/* Wait for Dobby to be done */



		/* Cleanup Global Variables here */
		pthread_mutex_destroy(&mutex);
		sem_destroy(&empty_list);
		sem_destroy(&full_list);
		sem_destroy(&cancel);
		free(dob);
		//free(p_tids);

		//printf("here\n");

		return 0;
}

/* Write the implementation of dobby() */

void* dobby( void * arg ) {
		pthread_t *p = arg;
		while(1){
				sem_wait(&empty_list);
				if(total_tasks == 0 && active_tasks == 0){
						//printf("here\n");
						sem_post(&full_list);
						break;	
				}

				else{
						//printf("total: %d\n", total_tasks);
						if(total_tasks >= 10){
								//printf("here\n");
								post_tasks(10);
								total_tasks -= 10;
								active_tasks += 10;
								sem_post(&full_list);
						}
						else{
								post_tasks(total_tasks);
								active_tasks += total_tasks;
								total_tasks = 0;
								sem_post(&full_list);
						}
				}
		}

		pthread_mutex_lock(&canc);
		for(int i = 0; i < NUM_ELVES; i++){
				//sleep(1);
				//printf("i: %d\n", i);
				//pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
				pthread_cancel(p[i]);
		}
		for(int k = 1; k <= NUM_ELVES; k++){
				sem_post(&cancel);
		}
		
		for(int j = 0; j < NUM_ELVES; j++){
				//printf("j: %d\n", j);
				pthread_join(p[j], NULL);
		}
	free(p);	
}  

/* Complete the implementation of house_elf() */

void* house_elf( void * ignore ) {
		/* The following function call registers the cleanup
		   handler to make sure that pthread_mutex_t locks 
		   are unlocked if the thread is cancelled. It must
		   be bracketed with a call to pthread_cleanup_pop 
		   so don't change this line or the other one */
		pthread_cleanup_push( house_elf_cleanup, NULL ); 
		while( 1 ) {
				pthread_mutex_lock(&mutex);
				task *t;
				//printf("total: %d active: %d\n", total_tasks, active_tasks);
				if(total_tasks == 0 && active_tasks == 0){
						pthread_mutex_unlock(&mutex);
						//pthread_mutex_unlock(&canc);
						break;
				}
				else{
						//pthread_mutex_lock(&mutex);
						if(active_tasks == 0){
								//printf("here\n");
								sem_post(&empty_list);
								sem_wait(&full_list);
						}
						t = take_task();
						active_tasks--;
						pthread_mutex_unlock(&mutex);
				}	
				do_work(t);
		}

		pthread_mutex_lock(&mutex);
		count++;
		if(count == 5){
				pthread_mutex_unlock(&canc);
				sem_post(&empty_list);
		}
		pthread_mutex_unlock(&mutex);	
		sem_wait(&cancel);
		while(1){
				//printf("made it\n");
				pthread_testcancel();
				//printf("pass\n");
		}
		/* This cleans up the registration of the cleanup handler */
		pthread_cleanup_pop( 0 ) ;
}

/* Implement unlocking of any pthread_mutex_t mutex types
   that are locked in the house_elf thread, if any 
   */
void house_elf_cleanup( void* arg ) {

		/*	
			if(pthread_mutex_trylock(&mutex) != 0){
			pthread_mutex_unlock(&mutex);

			}
			*/	   
}

/****** Do not change anything below this line -- Internal Functionality Only ********/ 

/* Generate random string code based off original by Ates Goral */
char* random_string(const int len, unsigned int * generator_seed) {
		static const char alphanum[] =
				"0123456789"
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz";
		char* s = malloc( (len+1) * sizeof( char ));

		for (int i = 0; i < len; ++i) {
				s[i] = alphanum[rand_r(generator_seed) % (sizeof(alphanum) - 1)];
		}
		s[len] = 0;
		return s;
}

void post_tasks( int howmany ) {
		printf("Adding %d tasks to the list of active tasks.\n", howmany);
		for ( int i = 0; i < howmany; ++i ) {
				task* t = malloc( sizeof( task ));
				t->id = ++task_id_counter;
				t->input = random_string( HASH_BUFFER_LENGTH, &random_seed );
				node* n = malloc( sizeof( node ));
				n->task = t;
				n->next = list_head;
				list_head = n;
		}
}

void do_work( task* todo ) {
		unsigned char* output_buffer = calloc( HASH_BUFFER_LENGTH , sizeof ( unsigned char ) );
		SHA256( todo->input, HASH_BUFFER_LENGTH, output_buffer );
		printf("Task %d was completed!\n", todo->id);
		free( output_buffer );
		free( todo->input );
		free( todo );
}

task* take_task( ) {
		if (list_head == NULL) {
				/* Something has gone wrong */
				printf("Trying to take from an empty list!\n");
				exit( -1 );
		}
		node* head = list_head;
		task* t = head->task;
		list_head = list_head->next;
		free( head );
		return t;
}
