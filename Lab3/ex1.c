#include <pthread.h>
#include <stdio.h>

// function prototype for thread 1
void* thread1();
// function prototype for thread 2
void* thread2();


void main() {
    pthread_t i1;   //thread 1 ID
    pthread_t i2;   //thread 2 ID

    /* Create two threads */
    pthread_create(&i1, NULL, (void*)&thread1, NULL);   //thread1 starts execution on thread1()
    pthread_create(&i2, NULL, (void*)&thread2, NULL);   //thread2 starts execution on thread2()

    /* main waits for the two threads to finish*/
    pthread_join(i1, NULL);
    pthread_join(i2, NULL);

    /* main's print */
    printf("Hello from main");

}
/* Thread1 function */
void* thread1() {
    sleep(5);   //sleep for 5 milisec once thread created
    printf("Hello from thread1");
}

/* Thread2 function */
void* thread2() {
    sleep(5); //sleep for 5 milisec once thread created
    printf("Hello from thread2");
}