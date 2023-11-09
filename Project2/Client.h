#ifndef STDIO
#define STDIO
#include <stdio.h>
#endif

#ifndef STDLIB
#define STDLIB
#include <stdlib.h>
#endif

#ifndef STRING
#define STRING
#include <string.h>
#endif

#ifndef PTHREAD
#define PTHREAD
#include <pthread.h>
#endif

#ifndef BANK
#define BANK
#include "Bank.h"
#endif

#ifndef TIME
#define TIME
#include <sys/time.h>
#endif


typedef struct trans
{               // structure for a transaction pair
    int acc_id; // account ID
    int amount; // amount to be added, could be positive or negative
}trans;

typedef struct request
{
    struct request *next;              // pointer to the next request in the list
    int request_id;                    // request ID assigned by the main thread
    int check_acc_id;                  // account ID for a CHECK request
    struct trans *transactions;        // array of transaction data
    int num_trans;                     // number of accounts in this transaction
    struct timeval starttime, endtime; // starttime and endtime for TIME
    char * request;
}request;

typedef struct queue
{
    struct request *head, *tail; // head and tail of the list
    int num_jobs;                // number of jobs currently in queue
    pthread_mutex_t queue_lock; // mutex lock for theQueue
}queue;

typedef struct account
{
    pthread_mutex_t lock;
    int acc_id;
    int balance;
}account;