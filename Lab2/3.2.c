#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "3.2.h"

int main() {
fork();
fork();
usleep(1);
printf("Process %d’s parent process ID is %d\n", getpid(), getppid());
sleep(2);
return 0;
}