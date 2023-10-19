#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void fpe_routine( );

int main() {

    int sixty = 60;
    int result = 0;
    int i = 0;

    signal(SIGFPE, fpe_routine);
    result = sixty / i;
    printf("%d\n", result);
    return 1;
}
void fpe_routine(int signo)
{
    printf("Caught a SIGFPE %d.\n", signo);
    exit(0);
}