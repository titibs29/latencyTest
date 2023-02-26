// create a thread that send a ping every second to another thread
// that will print the result of the ping
// the ping thread will also print the average, min, max and standard deviation
// of the ping

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mutex>

#define BILLION 1000000000L

// the thread function
void *ping(void *arg)
{
    // define the variables
    struct timespec start, end, waitTime;

    // set the wait time
    waitTime.tv_sec = 1;
    waitTime.tv_nsec = 0;

    // while loop
    while (1)
    {
        // get the start and end time
        clock_gettime(CLOCK_MONOTONIC, &start);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // wait loop
        while ((end.tv_sec - start.tv_sec) < waitTime.tv_sec && (end.tv_nsec - start.tv_nsec) < waitTime.tv_nsec)
        {
            clock_gettime(CLOCK_MONOTONIC, &end);
        }
        // send the result of the ping to the other thread

    }
        
}

// the second thread function
void *pong(void *arg)
{
    // while loop
    while (1)
    {
        // get the result of the ping from the other thread
        // print the result of the ping
    }
}

int main(int argc, char *argv[])
{
    // the thread identifier
    pthread_t thread;

    // create the first thread
    if (pthread_create(&thread, NULL, ping, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    // create the second thread
    if (pthread_create(&thread, NULL, pong, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

