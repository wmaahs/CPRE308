#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

main() {
execl("/bin/ls", "ls", NULL);
printf("What happened?\n");
}