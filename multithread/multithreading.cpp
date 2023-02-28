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
#include <cstring>

#define NUM_THREADS 2

// mutex to protect the ping variable
std::mutex ping_mutex;

// ping timespec queue
std::queue<std::chrono::time_point<std::chrono::high_resolution_clock>> ping_queue;

long long wait_time = 1000000000;   // 1 second
int rounds = 5;                     // 5 rounds


// ping thread
void *ping(void* )
{   
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    // get the start time
    start = std::chrono::high_resolution_clock::now();


    // add the first ping to the pingtime array
    ping_mutex.lock();
    ping_queue.push(start);
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (int i = 0; i < rounds; i++)
    {
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        // wait for the ping to be done
        end = std::chrono::high_resolution_clock::now();

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() < wait_time)
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
void* pong(void* )
{

    std::chrono::time_point<std::chrono::high_resolution_clock> actual, last;
    long long average = 0, min = LLONG_MAX, max = 0, std_dev = 0;
    long long *diffs = new long long [rounds];
    double percentage = 0;

    std::cout << "--- RT thread latency test for " << wait_time << " ns ---\n";

    // get the first ping time
    while (ping_queue.empty())
    {   
        // wait
        std::cout << "";
        
    }

    // get the ping time
    actual = ping_queue.front();
    ping_mutex.lock();
    ping_queue.pop();
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (int i = 0; i < rounds; i++)
    {   
        // fill the last ping time
        last = actual;

        // wait for the next ping   
        while (ping_queue.empty())
        {
            // wait
            std::cout << "";   

        }

        // get the ping time
        actual = ping_queue.back();
        ping_mutex.lock();
        ping_queue.pop();
        ping_mutex.unlock();

        // calculate diff
        diffs[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(actual-last).count()- wait_time;

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
    average /= rounds;

    // calculate the standard deviation
    for (int i = 0; i < rounds; i++)
    {
        std_dev += (diffs[i] - average) * (diffs[i] - average);
    }
    std_dev /= rounds;
    std_dev = sqrt(std_dev);

    // calculate the percentage
    percentage = (double)max / (double)wait_time * 100;


    // print the average, min, max and standard deviation, in the ping command style
    std::cout << "--- latency statistics ---\n";
    std::cout << rounds << " rounds completed" << "\n";
    std::cout << " min / max / avg / mdev (nanosecond) :\n";
    std::cout << min << " / " << max << " / " << average << " / " << std_dev << "\n";
    std::cout <<"max over sleep time percentage : " << percentage << "% \n";

    delete[] diffs;

    pthread_exit(NULL);

    return NULL;
}

int main( int argc, char *argv[])
{
    pthread_t threads[NUM_THREADS];
    int rc;

    // use the -t option to set the wait time, -r to set the number of ping
    // the default value are 1000000000 nanosecond and 5 ping
    if (argc > 1)
    {
        for (int i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-t") == 0)
            {
                wait_time = atoi(argv[i + 1]) * 1000000000L;
            }
            else if (strcmp(argv[i], "-r") == 0)
            {
                rounds = atoi(argv[i + 1]);
            }
            else if (strcmp(argv[i], "-h") == 0)
            {
                std::cout << "Usage : " << argv[0] << " [-t wait_time] [-r rounds]\n";
                std::cout << "wait_time is in nanosecond, rounds is the number of ping\n";
                std::cout << "default value are 1 second and 5 ping\n";
                exit(0);
            }
        }
    }


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
