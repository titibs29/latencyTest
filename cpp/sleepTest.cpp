#include <iostream>
#include <chrono>
#include <cmath>
#include <cstring>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif


#define BILLION  1000000000L
#define ROUNDS 5

int main(int argc, char *argv[])
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    long long diff;
    int i;
    long long diffs[ROUNDS];
    long long sum = 0;
    long long sleepTime = 10000L;            // sleep time in nanosecond

    // use the -t option to change the sleep time
    if (argc > 1)
    {
        if (strcmp(argv[1], "-t") == 0)
        {
            sleepTime = atol(argv[2]);
        }
    }

    printf("latency test for %lld ns\n", sleepTime);

    for (i = 0; i < ROUNDS; i++)
    {   
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        // sleep
        #ifdef _WIN32
        Sleep(sleepTime / 1000L);
        #else
        usleep(sleepTime / 1000L);
        #endif

        // get the end time
        end = std::chrono::high_resolution_clock::now();

        // calculate the difference in nanoseconds
        diff = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        // remove sleep time in nanosecond
        diff -= sleepTime;

        // print the difference
        printf("latency : %lld ns \n", diff);

        // store the difference
        diffs[i] = diff;
    }

    // calculate the average
    for (i = 0; i < ROUNDS; i++)
    {
        sum += diffs[i];
    }
    long long avg = sum / ROUNDS;

    // calculate ther min and max
    long long min = diffs[0];
    long long max = diffs[0];
    for (i = 0; i < ROUNDS; i++)
    {
        if (diffs[i] < min)
        {
            min = diffs[i];
        }
        if (diffs[i] > max)
        {
            max = diffs[i];
        }
    }

    // calculate the standard deviation
    long long sumOfSquares = 0;
    for (i = 0; i < ROUNDS; i++)
    {
        sumOfSquares += (diffs[i] - avg) * (diffs[i] - avg);
    }
    long long stdDev = sqrt(sumOfSquares / ROUNDS);

    // calculates the percentage of the average on the sleep time
    double percentage = ((double)avg * 100.0) / (double)sleepTime;


    // print the results
    printf(" --- latency results --- \n");
    printf(" %d rounds completed\n", ROUNDS);
    printf("latency min / max / avg / mdev (nanosecond) : %lld / %lld / %lld / %lld \n", min, max, avg, stdDev);
    printf("avg percentage over sleep time : %.3f %% \n", percentage);


}