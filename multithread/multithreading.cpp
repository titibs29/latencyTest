// create a thread that send a ping every second to another thread
// that will print the result of the ping
// the ping thread will also print the average, min, max and standard deviation
// of the ping

#include <pthread.h>
#include <ctime>
#include <mutex>
#include <iostream>
#include <queue>
#include <cmath>

#define NUM_THREADS 3
#define NUM_PING 5
#define WAIT_TIME_SEC 1
#define WAIT_TIME_NSEC 0

// mutex to protect the ping variable
std::mutex ping_mutex;

// ping timespec queue
std::queue<struct timespec> ping_queue;


// ping thread
void *ping(void *threadid)
{   
    struct timespec start, end;

    printf("ping thread started\n");

    // get the start time
    clock_gettime(CLOCK_REALTIME, &start);

    // add the first ping to the pingtime array
    ping_mutex.lock();
    ping_queue.push(start);
    ping_mutex.unlock();

    printf("ping thread setup done, entering loop\n");

    // loop until the number of ping is reached
    for (int i = 0; i < NUM_PING; i++)
    {
        // get the start time
        clock_gettime(CLOCK_REALTIME, &start);
        clock_gettime(CLOCK_REALTIME, &end);

        // wait for the ping to be done
        clock_gettime(CLOCK_REALTIME, &end);
        while (end.tv_sec - start.tv_sec < WAIT_TIME_SEC || end.tv_nsec - start.tv_nsec < WAIT_TIME_NSEC)
        {
            clock_gettime(CLOCK_REALTIME, &end);
        }

        // add the ping to the pingtime array
        ping_mutex.lock();
        ping_queue.push(end);
        ping_mutex.unlock();

        printf("ping %d sent\n", i);
    }

    printf("ping thread ended\n");

    pthread_exit(NULL);

    return NULL;

}

// pong thread
void *pong(void *threadid)
{
    printf("pong thread started\n");

    struct timespec actual, last, diff, diffs[NUM_PING];
    double average = 0;
    double min = 0;
    double max = 0;
    double std_dev = 0;

    printf("pong thread setup done\n");

    // get the first ping time
    while (ping_queue.empty())
    {

    }

    // get the ping time
    ping_mutex.lock();
    actual = ping_queue.front();
    ping_queue.pop();

    printf("first ping recieved, entering loop\n");
    
    // loop until the number of ping is reached
    for (int i = 0; i < NUM_PING; i++)
    {   
        // fill the last ping time
        last = actual;

        // wait for the ping to be done
        while (ping_queue.empty())
        {
    
        }

        printf("ping %d recieved\n", i);

        // get the ping time
        ping_mutex.lock();
        actual = ping_queue.front();
        ping_queue.pop();
        ping_mutex.unlock();

        // calculate diff
        diff.tv_sec = (actual.tv_sec - last.tv_sec) - WAIT_TIME_SEC;
        diff.tv_nsec = (actual.tv_nsec - last.tv_nsec) - WAIT_TIME_NSEC;

        // print the ping time
        std::cout << "Reply from ping : " << "Latency = " <<  diff.tv_nsec << "ns\n";

        // add the value to the average
        average += diff.tv_nsec;

        // add the value to the array
        diffs[i] = diff;

    }

    // calculate the average
    average /= NUM_PING;

    // calculate the min, max and standard deviation
    for (int i = 0; i < NUM_PING; i++)
    {
        // calculate the min
        if (diffs[i].tv_nsec < min)
        {
            min = diffs[i].tv_nsec;
        }

        // calculate the max
        if (diffs[i].tv_nsec > max)
        {
            max = diffs[i].tv_nsec;
        }

        // calculate the standard deviation
        std_dev += pow(diffs[i].tv_nsec - average, 2);
    }

    // print the average, min, max and standard deviation, in the ping command style
    std::cout << "--- latency statistics ---\n" << std::endl;
    std::cout << "    Packets: Sent = " << NUM_PING << ", Received = " << NUM_PING << ", Lost = 0 (0% loss)," << std::endl;
    std::cout << "Approximate round trip times in milli-seconds:" << std::endl;
    std::cout << "    Minimum = " << min << "ms, Maximum = " << max << "ms, Average = " << average << "ms" << std::endl;
    std::cout << "Standard deviation = " << std_dev << "ms" << std::endl;

    pthread_exit(NULL);

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int rc;

    printf("creating thread\n");

    // create the ping thread
    rc = pthread_create(&threads[0], NULL, ping, (void *)0);
    if (rc)
    {
        std::cout << "Error:unable to create thread," << rc << std::endl;
        exit(-1);
    }

    printf("ping thread created\n");

    // create the pong thread
    rc = pthread_create(&threads[1], NULL, pong, (void *)1);
    if (rc)
    {
        std::cout << "Error:unable to create thread," << rc << std::endl;
        exit(-1);
    }

    printf("pong thread created\n");

    // wait for the threads to finish
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    return 0;
}
