#include <stdlib.h>
#include "Bank.h"
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/types.h>
#include <pthread.h>
#include "appserver-coarse.h"
#include <threads.h>
#include <sys/syscall.h>

#define MAX_INPUT_SIZE 256

#define NUM_ARGS 4

int numWorkers, numAccounts;

int is_running = 1;

struct trans *transactions;
struct queue *theQueue;
struct account *accounts;

FILE *out_fp;

pthread_mutex_t parse_lock;

int worker_wait_cond = 0;
pthread_cond_t worker_wait;

pthread_mutex_t worker_wait_lock;

pthread_mutex_t account_lock;

/* PROTOTYPES */

/* prototype for runWorkers */
int runWorkers(int numWorkers);
/* prototype for func to handle thread execution*/
void *worker_handler();
/* protoype for starting appserver */
int appserver_start(int argc, char **argv);
/* prototype for halting server */

void print_incorrect_format();

int queue_init();

int client();

int push_theQueue(char *req_input, int req_id, struct timeval start_time);

request *pop_theQueue();

int main(int argc, char **argv)
{

    // Malloc theQueue
    theQueue = malloc(sizeof(queue));

    /* Initialize Server */
    if (appserver_start(argc, argv))
    {
        // error
        return -1;
    }
    accounts = malloc(numAccounts * sizeof(account));
    initialize_accounts(numAccounts);

    /* Init mutexs */
    if(pthread_mutex_init(&worker_wait_lock, NULL)){
        //error
        printf("Pthread failed to init");
    }
    if(pthread_mutex_init(&parse_lock, NULL)){
        //error
        printf("Pthread failed to init");
    }
    if(pthread_cond_init(&worker_wait, NULL)){
        //error
        printf("Pthread cond failed to init");
    }
    
    // /* Loop through accounts and init mutex's*/
    // for (int i = 0; i < numAccounts; i++)
    // {
    //     pthread_mutex_init(&accounts[i].lock, NULL);
    // }

    /* Init one account mux */
    pthread_mutex_init(&account_lock, NULL);

    /* Initialize the queue */
    if (queue_init())
    {
        // TODO
        //  failed to initialize the queue
        return -1;
    }

    if (client())
    {
        // error during client operations
        return -1;
    }

    for(int i = 0; i < numAccounts; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }

    free(theQueue);

    printf("Exiting...\n");
    return 0;
}

int appserver_start(int argc, char **argv)
{

    if (argc != NUM_ARGS)
    {
        // INCORRECT NUM OF ARGS
        fprintf(stderr, "Incorrect argument format\n");
        return -1;
    }
    if (!sscanf(argv[1], "%d", &numWorkers))
    {
        fprintf(stderr, "second argument needs to be the number of threads\n");
        return -1;
    }
    if (!sscanf(argv[2], "%d", &numAccounts))
    {
        fprintf(stderr, "third argument needs to be the number of accounts\n");
        return -1;
    }

    // open inputed file
    out_fp = fopen(argv[3], "w");

    if (!out_fp)
    {
        fprintf(stderr, "error (appserver): failed to open out_file\n");
        fprintf(stderr, "appserver: exiting program\n");
        return -1;
    }

    /* success */
    return 0;
}

void *end_server()
{
    /* TODO */
}

/* Function to ini*/
int queue_init()
{
    pthread_mutex_init(&(theQueue->queue_lock), NULL);
    theQueue->head = NULL;
    theQueue->tail = NULL;
    theQueue->num_jobs = 0;

    return 0;
}

/* Main loop that runs for the client to input requests */
int client()
{
    pthread_t workers[numWorkers];

    char *request_input = malloc(MAX_INPUT_SIZE * sizeof(char));

    /* request ID */
    int req_id = 1;

    struct timeval start_time;

    for (int i = 0; i < numWorkers; i++)
    {
        pthread_create(&workers[i], NULL, worker_handler, NULL);
    }

    while (1)
    {
        printf(">");
        /* Get input from client and null terminate */
        if (fgets(request_input, MAX_INPUT_SIZE, stdin) == NULL)
        {
            // shouldn't happen but probably will
            printf("fgets returned null");
            break;
        }
        request_input[strlen(request_input) - 1] = '\0';

        /* Check if END is inputed */
        if (strcmp(request_input, "END") == 0)
        {
            pthread_mutex_lock(&worker_wait_lock);
            worker_wait_cond = 1;
            is_running = 0;
            pthread_cond_broadcast(&worker_wait);
            pthread_mutex_unlock(&worker_wait_lock);
            printf("EXIT was sent...\n");
            for (int i = 0; i < numWorkers; i++)
            {
                pthread_join(workers[i], NULL);
                // For Testing
                printf("thread %d joined\n", i);
            }
            fclose(out_fp);
            break;
        }

        /* Create request and
         Add request to theQueue */
        gettimeofday(&start_time, NULL);
        if (push_theQueue(request_input, req_id, start_time))
        {
            // TODO
            req_id++;
            pthread_cond_broadcast(&worker_wait);
        }
        else
        {
            // failed to push the queue
            break;
        }
    }
    free(request_input);
}

void *worker_handler()
{
    /* TODO */
    /* The current request */
    request *curr_req = malloc(MAX_INPUT_SIZE * sizeof(request));

    char **req_str = malloc(MAX_INPUT_SIZE * sizeof(char *));
    char *request_splitter;

    int num_args = 0;

    int check_accnt;

    int check_amnt;

    // flag to mark ISF
    int ISF = 0;

    while (is_running || theQueue->num_jobs > 0)
    {
        pthread_mutex_lock(&worker_wait_lock);
        while (theQueue->num_jobs == 0 && !worker_wait_cond)
        {
            // // For Testing
            // printf("Worker Waiting.. NumJobs: %d, pid: %d\n", theQueue->num_jobs, syscall(__NR_gettid));
            pthread_cond_wait(&worker_wait, &worker_wait_lock);

            //Check for END
        }

        if (!is_running && theQueue->num_jobs == 0)
        {
            pthread_mutex_unlock(&worker_wait_lock);
            pthread_exit(NULL);
            return NULL;
        }
        // // For Testing
        // printf("Done Waiting.. NumJobs: %d, pid: %d\n", theQueue->num_jobs, syscall(__NR_gettid));

        curr_req = pop_theQueue();
        pthread_mutex_unlock(&worker_wait_lock);

        // // // For Testing
        // printf("POPPED.. NumJobs: %d ReqId: %d PID: %d\n", theQueue->num_jobs, curr_req->request_id, syscall(__NR_gettid));

        /*
         *Parse the command
         * lock the curr command from other threads
         */
        pthread_mutex_lock(&parse_lock);
        request_splitter = strtok(curr_req->request, " ");
        while (request_splitter)
        {
            req_str[num_args] = malloc(MAX_INPUT_SIZE * sizeof(char));
            strncpy(req_str[num_args], request_splitter, MAX_INPUT_SIZE);
            num_args++;
            request_splitter = strtok(NULL, " ");
        }
        // Unlcok mutext for parsing curr command
        pthread_mutex_unlock(&parse_lock);

        /*
         * Check for different Commands
         * Check for Check Command
         */
        if ((strcmp(req_str[0], "CHECK") == 0) && (num_args == 2))
        {

            pthread_mutex_lock(&parse_lock);
            check_accnt = atoi(req_str[1]);
            pthread_mutex_lock(&account_lock);    //change here
            check_amnt = read_account(check_accnt);
            gettimeofday(&curr_req->endtime, NULL);
            pthread_mutex_unlock(&account_lock);  //change here
            flockfile(out_fp);
            fprintf(out_fp, "%d BAL %d TIME %d.%06d %d.%06d\n",
                    curr_req->request_id, check_amnt, curr_req->starttime.tv_sec,
                    curr_req->starttime.tv_usec, curr_req->endtime.tv_sec, curr_req->endtime.tv_usec);
            funlockfile(out_fp);
            pthread_mutex_unlock(&parse_lock);
        }

        // TRANSACTION
        // Stil need to write to file
        else if ((strcmp(req_str[0], "TRANS") == 0) && num_args > 2)
        {

            // //For testing
            // printf("TRANS started\n");
            ISF = 0;

            int num_of_accounts = (num_args - 1) / 2;

            int trans_accnt[num_of_accounts];
            int trans_amnt[num_of_accounts];
            int trans_prev_balance[num_of_accounts];
            int sort_temp;

            pthread_mutex_lock(&parse_lock);
            for (int i = 0; i < num_of_accounts; i++)
            {
                trans_accnt[i] = atoi(req_str[i * 2 + 1]);
                // // For Testing
                // printf("trans accnt: %d ", trans_accnt[i]);
                trans_amnt[i] = atoi(req_str[i * 2 + 2]);
                // // For Testing
                // printf("trans amnt: %d\n", trans_amnt[i]);
            }
            // unlock req string
            pthread_mutex_unlock(&parse_lock);

            // sort trans by account number
            for (int i = 0; i < num_of_accounts; i++)
            {
                for (int j = i; j < num_of_accounts; j++)
                {
                    if (trans_accnt[j] < trans_accnt[i])
                    {
                        // Swap trans accounts
                        sort_temp = trans_accnt[i];
                        trans_accnt[i] = trans_accnt[j];
                        trans_accnt[j] = sort_temp;
                        // Swap trans amnts
                        sort_temp = trans_amnt[i];
                        trans_amnt[i] = trans_amnt[j];
                        trans_amnt[j] = sort_temp;
                    }
                }
            }
            
            // Lock entire bank
            pthread_mutex_lock(&account_lock); //change here
            

            // Check for ISF flags
            for (int i = 0; i < num_of_accounts; i++)
            {
                trans_prev_balance[i] = read_account(trans_accnt[i]);
                if (trans_prev_balance[i] + trans_amnt[i] < 0)
                {
                    gettimeofday(&curr_req->endtime, NULL);
                    if(curr_req->endtime.tv_usec <curr_req->starttime.tv_usec) {
                        gettimeofday(&curr_req->endtime, NULL);
                    }
                    flockfile(out_fp);
                    fprintf(out_fp, "%d ISF %d TIME %d.06%d %d.06%d\n",
                                curr_req->request_id, trans_accnt[i],
                                curr_req->starttime.tv_sec, curr_req->starttime.tv_usec,
                                curr_req->endtime.tv_sec, curr_req->endtime.tv_usec);
                    funlockfile(out_fp);
                    ISF = 1;
                    break;
                }
            }

            //if no ISF detected
            if (!ISF)
            {
                //complete transactions
                for (int i = 0; i < num_of_accounts; i++)
                {
                    write_account(trans_accnt[i], trans_prev_balance[i] + trans_amnt[i]);
                }

                // finished trans write to file
                gettimeofday(&curr_req->endtime, NULL);
                flockfile(out_fp);
                fprintf(out_fp, "%d OK TIME %d.%06d %d.%06d\n",
                    curr_req->request_id, curr_req->starttime.tv_sec,
                    curr_req->starttime.tv_usec, curr_req->endtime.tv_sec,
                    curr_req->endtime.tv_usec);
                funlockfile(out_fp);
            }
            // Unlock entire bank
            pthread_mutex_unlock(&account_lock);   //change here
        }
        else
        {
            // TODO
            fprintf(stderr, "Check your spelling\nYou typed %s\n", req_str[0]);
        }
        for (int i = 0; i < num_args; i++)
        {
            free(req_str[i]);
        }
        num_args = 0;

        free(curr_req->request);
    }
    free(req_str);
    free(curr_req);

}

void print_incorrect_format()
{
    fprintf(stderr, "ERROR: You did not input the command correctly\n");
    fprintf(stderr, "Exiting...\n");
}

int push_theQueue(char *req_input, int req_id, struct timeval start_time)
{
    printf("< ID %d\n", req_id);
    request *new_req = (struct request *)malloc(sizeof(request));
    new_req->request = (char *)malloc(MAX_INPUT_SIZE * sizeof(char));

    gettimeofday(&(new_req->starttime), NULL);
    new_req->request_id = req_id;
    strncpy(new_req->request, req_input, MAX_INPUT_SIZE);

    new_req->next = NULL;

    pthread_mutex_lock(&(theQueue->queue_lock));


    // If the queue has jobs
    if (theQueue->num_jobs > 0)
    {
        // set tail to to new requestC
        theQueue->tail->next = new_req;
        theQueue->tail = new_req;
        // increase size of queue
        theQueue->num_jobs = theQueue->num_jobs + 1;
        // unlock mutex
        pthread_mutex_unlock(&(theQueue->queue_lock));

        // return success
        return 1;
    }
    // if the queue is empty
    else if (theQueue->num_jobs == 0)
    {
        // set head and tail to command
        theQueue->head = new_req;
        theQueue->tail = new_req;
        theQueue->num_jobs = 1;
        // unlock mutex
        pthread_mutex_unlock(&(theQueue->queue_lock));

        // return success
        return 1;
    }
    // should never happen (queueu size is negative)
    else
    {
        fprintf(stderr, "Failed to push to the queue\n");
        pthread_mutex_unlock(&(theQueue->queue_lock));

        return 0;
    }
}

request *pop_theQueue()
{

    request *old_head;
    request *ret_req = malloc(MAX_INPUT_SIZE * sizeof(request));

    /* Check the size of theQueue
     *to make sure there are req
     */
    if (theQueue->num_jobs > 0)
    {
        // if queue is not empty

        // set ret_req = head
        ret_req->request_id = theQueue->head->request_id;
        ret_req->request = malloc(MAX_INPUT_SIZE * sizeof(char));
        ret_req->starttime = theQueue->head->starttime;
        strncpy(ret_req->request, theQueue->head->request, MAX_INPUT_SIZE);
        ret_req->next = NULL;

        // Free old head and update address to new head
        old_head = theQueue->head;
        theQueue->head = theQueue->head->next;
        free(old_head->request);
        free(old_head);

        // Check for overflow
        if (!theQueue->head)
        {
            theQueue->tail = NULL;
        }

        theQueue->num_jobs = theQueue->num_jobs - 1;

    }
    // if theQueue is empty
    else
    {
        ret_req->request = NULL;
        pthread_mutex_unlock(&(theQueue->queue_lock));
        return ret_req;
    }

    return ret_req;
}