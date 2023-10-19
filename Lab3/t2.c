/* t2.c
   synchronize threads through mutex and conditional variable 
   To compile use: gcc -o t2 t2.c -lpthread 
*/ 

#include <stdio.h>
#include <pthread.h>

void* 	hello();    // define two routines called by threads    
void* 	world();
void*   again();    //define third routine called by third thread         	

/* global variable shared by threads */
pthread_mutex_t 	mutex;  		// mutex
pthread_cond_t 		done_hello; 	// conditional variable
pthread_cond_t      done_world;     // new conditional variable for synch
int 			done_0 = 0;      	// testing variable
int             done_1 = 0;     // new testing variable

int main (int argc, char *argv[]){
    pthread_t 	tid_hello, // thread id  
    		tid_world, tid_again; // second and third thread id
    /*  initialization on mutex and cond variable  */ 
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&done_hello, NULL); 
    pthread_cond_init(&done_world, NULL);

    pthread_create(&tid_hello, NULL, (void*)&hello, NULL); //thread creation
    pthread_create(&tid_world, NULL, (void*)&world, NULL); //thread creation
    pthread_create(&tid_again, NULL, (void*)&again, NULL); //third thread creation 

    /* main waits for the two threads to finish */
    pthread_join(tid_hello, NULL);  
    pthread_join(tid_world, NULL);

    /* main thread also waits for third thread*/
    pthread_join(tid_again, NULL);

    printf("\n");
    return 0;
}

void* hello() {
    pthread_mutex_lock(&mutex);
    printf("Hello ");
    fflush(stdout); 	// flush buffer to allow instant print out
    done_0 = 1;
    pthread_cond_signal(&done_hello);	// signal world() thread
    pthread_mutex_unlock(&mutex);	// unlocks mutex to allow world to print
    return ;
}


void* world() {
    pthread_mutex_lock(&mutex);

    /* world thread waits until done_0 == 1. */
    while(done_0 == 0)  //changed name of first done flag
	pthread_cond_wait(&done_hello, &mutex);

    printf("World ");
    fflush(stdout);
    done_1 = 1;     //flag for world()
    pthread_cond_signal(&done_world);       //conditional variable for third routine
    pthread_mutex_unlock(&mutex); // unlocks mutex

    return ;
}

/* third routine for thread three */
void* again(){
    pthread_mutex_lock(&mutex);

    /* again thread waits for done_1 ==1 */
    while(done_1 == 0)
    pthread_cond_wait(&done_world, &mutex); // listen for signal from world()

    printf("Again!");    //print statment
    fflush(stdout);     //flush buffer
    pthread_mutex_unlock(&mutex);       //unlock mutex

    return ;
}
