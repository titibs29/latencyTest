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

int main(int argc, char *argv[])
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    long long diff;
    int i, rounds = 5;
    long long sum = 0;
    long long sleepTime = 1000000000L;            // sleep time in nanosecond

    // use the -t option to change the sleep time or use the -r option to change the number of rounds
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-t") == 0)
            {
                long long temp = atoll(argv[i+1]);
                if (temp > 0)
                {
                    sleepTime = temp;
                }
            }
            else if (strcmp(argv[i], "-r") == 0)
            {
                rounds = atoi(argv[i+1]);
            }
            else if (strcmp(argv[i], "-h") == 0){
                printf("usage : %s [-t sleepTime] [-r rounds] \n", argv[0]);
                return 0;
            }
        }
    }

    long long *diffs = new long long[rounds];

    printf("latency test for %lld ns\n", sleepTime);

    for (i = 0; i < rounds; i++)
    {   
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        // sleep
        #ifdef _WIN32
        Sleep(sleepTime / 1000000L);
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
    for (i = 0; i < rounds; i++)
    {
        sum += diffs[i];
    }
    long long avg = sum / rounds;

    // calculate ther min and max
    long long min = diffs[0];
    long long max = diffs[0];
    for (i = 0; i < rounds; i++)
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
    for (i = 0; i < rounds; i++)
    {
        sumOfSquares += (diffs[i] - avg) * (diffs[i] - avg);
    }
    long long stdDev = sqrt(sumOfSquares / rounds);

    // calculates the percentage of the average on the sleep time
    double percentage = ((double)avg * 100.0) / (double)sleepTime;


    // print the results
    printf(" --- latency results --- \n");
    printf(" %d rounds completed\n", rounds);
    printf("latency min / max / avg / mdev (nanosecond) : %lld / %lld / %lld / %lld \n", min, max, avg, stdDev);
    printf("avg percentage over sleep time : %.3f %% \n", percentage);

    delete[] diffs;

}