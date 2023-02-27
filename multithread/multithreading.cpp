// create a thread that send a ping every second to another thread
// that will print the result of the ping
// the ping thread will also print the average, min, max and standard deviation
// of the ping

#include <cmath>
#include <chrono>
#include <pthread.h>
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
void *ping(void *threadid)
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
        ping_mutex.unlock();
    }

    pthread_exit(NULL);

    return NULL;

}

// pong thread
void *pong(void *threadid)
{

    std::chrono::time_point<std::chrono::high_resolution_clock> actual, last;
    long long average = 0, min = LLONG_MAX, max = 0, std_dev = 0;
    long long diffs[NUM_PING];

    // get the first ping time
    while (ping_queue.empty())
    {

    }

    // get the ping time
    ping_mutex.lock();
    actual = ping_queue.front();

    while(!ping_queue.empty())
    {
        ping_queue.pop();

    }
    // ping_queue.pop();
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (int i = 0; i < NUM_PING; i++)
    {   
        // fill the last ping time
        last = actual;
        

        // wait for the next ping
        while (ping_queue.empty())
        {
            // wait

        }

        // get the ping time
        ping_mutex.lock();
        actual = ping_queue.back();
        while(!ping_queue.empty())
        {
            ping_queue.pop();
        }
        ping_mutex.unlock();

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


    // print the average, min, max and standard deviation, in the ping command style
    std::cout << "--- latency statistics ---\n";
    std::cout << "Packets: Sent = " << NUM_PING << ", Lost = 0 (0% loss),\n";
    std::cout << "Approximate round trip times in milli-seconds:\n";
    std::cout << "    Minimum = " << min << "ms, Maximum = " << max << "ms, Average = " << average << "ms\n";
    std::cout << "Standard deviation = " << std_dev << "ms\n";

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

    // create the pong thread
    rc = pthread_create(&threads[1], NULL, pong, (void *)1);
    if (rc)
    {
        std::cout << "Error:unable to create thread," << rc << "\n";
        exit(-1);
    }

    // wait for the threads to finish
    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    return 0;
}
