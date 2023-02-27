// create a thread that send a ping every second to another thread
// that will print the result of the ping
// the ping thread will also print the average, min, max and standard deviation
// of the ping

#include <cmath>
#include <chrono>
#include <pthread.h>
#include <climits>
#include <iostream>
#include <mutex>
#include <queue>

#define NUM_THREADS 2
#define NUM_PING 5
#define WAIT_TIME 1000000000L

// mutex to protect the ping variable
std::mutex ping_mutex;

// ping timespec queue
std::queue<std::chrono::time_point<std::chrono::high_resolution_clock>> ping_queue;


// ping thread
void *ping(void * )
{   
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    // get the start time
    start = std::chrono::high_resolution_clock::now();


    // add the first ping to the pingtime array
    ping_mutex.lock();
    ping_queue.push(start);
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (int i = 0; i < NUM_PING; i++)
    {
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        // wait for the ping to be done
        end = std::chrono::high_resolution_clock::now();

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() < WAIT_TIME)
        {
            end = std::chrono::high_resolution_clock::now();
        }

        // add the ping to the ping time array
        ping_mutex.lock();
        ping_queue.push(end);
        // print ping_queue.empty
        ping_mutex.unlock();
    }

    pthread_exit(NULL);

    return NULL;

}

// pong thread
void* pong(void*)
{

    std::chrono::time_point<std::chrono::high_resolution_clock> actual, last;
    long long average = 0, min = LLONG_MAX, max = 0, std_dev = 0;
    long long diffs[NUM_PING];
    double percentage = 0;
    volatile char nullwait = 0;         // volatile to prevent the compiler to optimize the loop
    bool isEmpty = false;

    std::cout << "latency test for " << WAIT_TIME << " ns\n";

    // get the first ping time
    isEmpty = ping_queue.empty();

    while (isEmpty)
    {   
        // wait
        isEmpty = ping_queue.empty();    
    }

    // get the ping time
    actual = ping_queue.front();
    ping_mutex.lock();
    ping_queue.pop();
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (int i = 0; i < NUM_PING; i++)
    {   
        // fill the last ping time
        last = actual;

        printf("wait for the next ping \n");

        // wait for the next ping
        isEmpty = ping_queue.empty();    
        while (ping_queue.empty())
        {
            // wait
            isEmpty = ping_queue.empty();    
        }

        printf("next ping received \n");

        // get the ping time
        actual = ping_queue.back();
        ping_mutex.lock();
        ping_queue.pop();
        ping_mutex.unlock();
    

        std::cout << "actual ping time : " << std::chrono::duration_cast<std::chrono::nanoseconds>(actual.time_since_epoch()).count() << "\n";

        // calculate diff
        diffs[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(actual-last).count()- WAIT_TIME;

        // print the ping time
        std::cout << "Reply from ping : Latency = " << diffs[i] << "ns\n";

        // add the value to the average
        average += diffs[i];

        // update the min and max
        if (diffs[i] < min)
        {
            min = diffs[i];
        }
        if (diffs[i] > max)
        {
            max = diffs[i];
        }

    }

    // calculate the average
    average /= NUM_PING;

    // calculate the standard deviation
    for (int i = 0; i < NUM_PING; i++)
    {
        std_dev += (diffs[i] - average) * (diffs[i] - average);
    }
    std_dev /= NUM_PING;
    std_dev = sqrt(std_dev);

    // calculate the percentage
    percentage = (double)max / (double)WAIT_TIME * 100;


    // print the average, min, max and standard deviation, in the ping command style
    std::cout << "--- latency statistics ---\n";
    std::cout << "%d rounds completed" << NUM_PING << "\n";
    std::cout << " min / max / avg / mdev (nanosecond) :\n";
    std::cout << min << " / " << max << " / " << average << " / " << std_dev << "\n";
    std::cout <<"max over sleep time percentage : " << percentage << "% \n";

    pthread_exit(NULL);

    return NULL;
}

int main()
{
    pthread_t threads[NUM_THREADS];
    int rc;

    // create the ping thread
    rc = pthread_create(&threads[0], NULL, ping, (void *)0);
    if (rc)
    {
        std::cout << "Error:unable to create thread," << rc << "\n";
        exit(-1);
    }
    std::cout << "ping thread created\n";

    // create the pong thread
    rc = pthread_create(&threads[1], NULL, pong, (void *)1);
    if (rc)
    {
        std::cout << "Error:unable to create thread," << rc << "\n";
        exit(-1);
    }

    std::cout << "pong thread created\n";

    // wait for the threads to finish
    pthread_join(threads[0], NULL);

    std::cout << "ping thread finished\n";

    pthread_join(threads[1], NULL);

    std::cout << "threads finished\n";

    return 0;
}
