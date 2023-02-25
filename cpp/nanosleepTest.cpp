#include <iostream>
#include <cstring>
#include <climits>
#include <cmath>


#define BILLION  1000000000L

int main(int argc, char *argv[])
{
    struct timespec start, end;
    long long diff = 0;
    int i,rounds = 5;

    // allocate memory for the differences
    long long *diffs = new long long[rounds];
    long long sum = 0;
    struct timespec sleepTime = {1,0}; // sleep time in nanosecond

    // use the -t option to change the sleep time or use the -r option to change the number of rounds
    if (argc > 1)
    {
        for (i = 1; i < argc; i++)
        {
            if (strcmp(argv[i], "-t") == 0)
            {
                long long temp = atoll(argv[i+1]);
                sleepTime.tv_sec = temp / BILLION;
                sleepTime.tv_nsec = temp % BILLION;
            }
            else if (strcmp(argv[i], "-r") == 0)
            {
                rounds = atoi(argv[i+1]);
                delete[] diffs;
                diffs = new long long[rounds];
            }
            else if (strcmp(argv[i], "-h") == 0){
                printf("usage : %s [-t sleepTime] [-r rounds] \n", argv[0]);
                return 0;
            }

        }
    }

    printf("latency test for %ld s %ld ns\n", sleepTime.tv_sec, sleepTime.tv_nsec);

    for (i = 0; i < rounds; i++)
    {   
        // get the start time
        clock_gettime(CLOCK_MONOTONIC, &start);

        // precise sleep
        nanosleep(&sleepTime, NULL);

        // get the end time
        clock_gettime(CLOCK_MONOTONIC, &end);

        // calculate the difference in nanoseconds
        diff = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);

        // remove sleep time in nanosecond
        diff -= (BILLION * sleepTime.tv_sec) + sleepTime.tv_nsec;

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
    long long max = LONG_LONG_MIN;
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
    double percentage = ((double)avg * 100.0) / (double)(BILLION * sleepTime.tv_sec) + (double)sleepTime.tv_nsec;


    // print the results
    printf(" --- latency results --- \n");
    printf(" %d rounds completed\n", rounds);
    printf("latency min / max / avg / mdev (nanosecond) : %lld / %lld / %lld / %lld \n", min, max, avg, stdDev);
    printf("avg percentage over sleep time : %.3f %% \n", percentage);

    // free the memory
    delete[] diffs;

    return 0;

}