#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"

int main(int argc, char** argv) {
    printf("Process ID  is: %d\n", getpid());
    printf("Parent process ID is: %d\n", getppid());
    sleep(120); /* sleeps for 2 minutes */
    printf("I am awake.\n");
    return 0;
}