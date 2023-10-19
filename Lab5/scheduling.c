/*******************************************************************************
 * *
 * * CprE 308 Scheduling Lab
 * *
 * * scheduling.c
 * * last updated 2020-03-01 - mlw
 * *******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_PROCESSES 20
#define NUM_SCHEDULERS 4

#define SCHEDULER_NAME_LEN 30

/* Defines a simulated process */
typedef struct process {
  int arrivaltime;    /* Time process arrives and wishes to start */
  int starttime;      /* Time process actually starts */
  int runtime;        /* Time process requires to complete job */
  int remainingtime;  /* Runtime remaining before process completes */
  int endtime;        /* Time process finishes */
  
  int priority;       /* Priority of the process */

  int running;        /* Convenience - indicates if the process is currently running */
  int finished;       /* Convenience - indicates if the process has finished */
} process;

/* Defines a simulated scheduler */
typedef struct scheduler
{
  int (* func) (process [], int); /* Function to run for the scheduler */
  char name[SCHEDULER_NAME_LEN];  /* Name of the scheduler, for pretty printing */
} scheduler;

/* Prototypes of scheduler functions */
int compare_arrival(const void *a, const void *b);
int highest_pending_priority(struct process proc[], int t);
int first_come_first_served(process proc[], int t);
int shortest_remaining_time(process proc[], int t);
int round_robin(process proc[], int t);
int round_robin_priority(process proc[], int t);

/* Main, contains most of the simulation code */
int main() {
  int i,j;
  process proc[NUM_PROCESSES],        /* List of processes */
            proc_copy[NUM_PROCESSES]; /* Backup copy of processes */
            
  /* Initialize the schedulers */
  scheduler schedulers[4];
  schedulers[0].func = first_come_first_served;
  strncpy(schedulers[0].name, "First come first served", SCHEDULER_NAME_LEN);
  schedulers[1].func = shortest_remaining_time;
  strncpy(schedulers[1].name, "Shortest remaining time", SCHEDULER_NAME_LEN);
  schedulers[2].func = round_robin;
  strncpy(schedulers[2].name, "Round robin", SCHEDULER_NAME_LEN);
  schedulers[3].func = round_robin_priority;
  strncpy(schedulers[3].name, "Round robin with priority", SCHEDULER_NAME_LEN);

  /* Seed random number generator */
  // srand(time(0));    /* Use this seed to test different inputs */
  srand(0xC0FFEE);     /* Use this seed to test fixed input */

  /* Initialize process structures */
  for(i=0; i<NUM_PROCESSES; i++)
  {
    proc[i].arrivaltime = rand() % 100;
    proc[i].runtime = (rand() % 30) + 10;
    proc[i].remainingtime = proc[i].runtime;
    proc[i].priority = rand() % 3;
	
    proc[i].starttime = -1;
    proc[i].endtime = -1;
    proc[i].running = 0;
    
    proc[i].finished = 0;
  }

  /* Print process values */
  printf("Process\tarrival\truntime\tpriority\n");
  for(i = 0; i < NUM_PROCESSES; i++) {
    printf("%d\t%d\t%d\t%d\n", i, proc[i].arrivaltime, proc[i].runtime,
           proc[i].priority);
  }
  
  /* Run simulation for each scheduler */
  for(i = 0; i < NUM_SCHEDULERS; i++) {
    int num_finished = 0;
    int current_time = 0;
    int prev_pid = -1;
    int total_turnaround_time = 0;
    printf("\n\n--- %s\n", schedulers[i].name);
    
    /* Use a copy of the processes array */
    memcpy(proc_copy, proc, NUM_PROCESSES * sizeof(process));
    
    /* Run the simulation until all processes finish */
    while(num_finished < NUM_PROCESSES) {
      process * p;
      
      /* Call the scheduler */
      int next_pid = schedulers[i].func(proc_copy, current_time);
      
      /* If there's no process to run, just move time forward */
      if(next_pid < 0) {
        current_time += 1;
        continue;
      }
      
      /* Convenience - use p to reference the proc */
      p = &proc_copy[next_pid];
      
      /* If the process just started, print a message */
      if(p->starttime == -1) {
        printf("%03d: Process %d started\n", current_time, next_pid);
        p->starttime = current_time;
      }
      
      /* Update which process is running */
      if(prev_pid >= 0) {
        proc_copy[prev_pid].running = 0;
      }
      prev_pid = next_pid;
      p->running = 1;
      
      /* Move time forward */
      current_time += 1;
      
      /* Update remaining time of the process that just ran */
      p->remainingtime -= 1;
      
      /* If the process already finished, it shouldn't have been chosen */
      if(p->remainingtime < 0) {
        printf("Error: tried to run finished process %d\n", next_pid);
        continue;
      /* If the process just finished, print a message and do bookkeeping */
      } else if(p->remainingtime == 0) {
        printf("%03d: Process %d finished\n", current_time, next_pid);
        p->endtime = current_time;
        p->finished = 1;
        num_finished += 1;
        /* Keep a running total of turnaround time */
        total_turnaround_time += p->endtime - p->arrivaltime;
      }
    }
    
    /* Print average turnaround time */
    printf("Average turnaround time: %.01lf seconds\n", (double)total_turnaround_time / NUM_PROCESSES);
  }

  return 0;
}

/* ----------------------Schedulers------------------------------*/
/* The scheduler being simulated is called each timestep.
   The scheduler function should return the index of the process 
   from proc[] that should run next. Parameter t is the current time 
   of the simulation. Note that proc[] contains all processes, 
   including those that have not yet arrived and those that are finished.
   You are responsible for ensuring that you schedule a "ready"
   process. If no processes are ready, return -1.
*/

int first_come_first_served(process proc[], int t)
{
  
  process org_proc[NUM_PROCESSES];
  process next;
  int next_ind = 0;

  /* Sort proc array into copy organized by arrival time */
  for(int i = 0; i <NUM_PROCESSES; i++) {
    org_proc[i] = proc[i];
  }

  /* Use qsort and compare arrival function to sort array */
  qsort(org_proc, NUM_PROCESSES, sizeof(process), compare_arrival);
  
  /* Scan through sorted array and find out if top proc has finished.
    If the process has not finished then send proc index as next pid.
    If process has finished, go to next proc in sorted proc array.
   */
  for(int i = 0; i <NUM_PROCESSES; i++) {
    if(!org_proc[i].finished) {
      next = org_proc[i];
      /* Check if first proc has arrived yet */
      if(next.arrivaltime > t) {
        return -1;
      }
      for(int j = 0; j < NUM_PROCESSES; j++) {
        if((next.arrivaltime == proc[j].arrivaltime) && (next.runtime == proc[j].runtime)){
          next_ind = j;
        }
      }
      break;
    }
  }
  return next_ind;
}

int shortest_remaining_time(process proc[], int t)
{
  int shortest_time = __INT32_MAX__;
  int shortest_ind = -1;

  for (int i = 0; i < NUM_PROCESSES; i++)
  {
    /* Check if proc has shortest remaining time.
      On the event of a tie for shortest remaining time,
      tie goes to smallest index */
    if ((proc[i].remainingtime < shortest_time) && (proc[i].remainingtime != 0))
    {
      /* Check if process is "ready" */
      if (t >= proc[i].arrivaltime)
      {
        shortest_time = proc[i].remainingtime;  //set shortest time
        shortest_ind = i;   //set index of shortest time
      }
    }
  }
  return shortest_ind;
}

int round_robin(process proc[], int t)
{
  /* TODO: Implement scheduling algorithm here */
  // HINT: consider using a static variable to keep track of the previously scheduled process
  static int current_index = 0; // Static variable to remember the previous state

  // Search for the next process that's "ready" and not finished, starting from the next process after the current one
  for (int i = 0; i < NUM_PROCESSES; i++)
  {
    int index = (current_index + i) % NUM_PROCESSES; // Using modulo to wrap around the array

    // Check if the process has arrived and not finished
    if (proc[index].arrivaltime <= t && !proc[index].finished)
    {
      current_index = (index + 1) % NUM_PROCESSES; // Move to the next process for the next call
      return index;                                // Return the current process's index
    }
  }

  // If no process found that's "ready" and not finished, return -1
  return -1;
}

int round_robin_priority(process proc[], int t)
{
    static int current_index = 0;

    int current_priority = highest_pending_priority(proc, t);

    // Search for the next process that's "ready", not finished, and matches the highest priority
    for (int i = 0; i < NUM_PROCESSES; i++) {
        int index = (current_index + i) % NUM_PROCESSES;

        if (proc[index].arrivaltime <= t && !proc[index].finished && proc[index].priority == current_priority) {
            current_index = (index + 1) % NUM_PROCESSES;
            return index;
        }
    }

    // If no suitable process found, return -1
    return -1;
}

int compare_arrival(const void *a, const void *b) {
    return ((process *)a)->arrivaltime - ((process *)b)->arrivaltime;
}

int highest_pending_priority(struct process proc[], int t) {
    int max_priority = -1; // Initialize with a value that is lower than any actual priority
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (proc[i].arrivaltime <= t && !proc[i].finished && proc[i].priority > max_priority) {
            max_priority = proc[i].priority;
        }
    }
    return max_priority;
}
