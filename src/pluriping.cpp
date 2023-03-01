#include <cmath>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <queue>
#include <chrono>
#include <thread>
#include <mutex>


/// @brief ping function, ping every [sleep_time] ns and stock the ping time in the ping_queue and the ping id in the ping_id_queue
/// @param tid id of the thread
/// @param rounds number of ping sent
/// @param sleep_time wait time between each ping
/// @param ping_mutex mutex to protect the ping variable
/// @param ping_queue queue to stock the ping time
/// @param ping_id_queue queue to stock the ping id
void ping(unsigned int tid, unsigned int rounds, long long unsigned int sleep_time, std::mutex &ping_mutex, std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> &ping_queue, std::queue <unsigned int> &ping_id_queue);


/// @brief main function
/// @param argc argument count
/// @param argv argument vector
/// @return 
int main(int argc, char *argv[])
{

    //define variables
    // system variables
    long long unsigned int id = 0, i = 0, j = 0;
    long long unsigned int diff = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    // user defined variables
    long long unsigned int rounds = 5;              // number of ping sent
    long long unsigned int sleep_time = 1000000000; // wait time between each ping in ns
    long long unsigned int nb_threads = 2;          // number of threads



    // use the -t option to set the wait time, -r to set the number of ping, -n to set the number of threads
    if(argc > 1)
    {
        for(i = 1; i < static_cast<unsigned int>(argc); i++)
        {
            // the t option set the wait time
            if(strcmp(argv[i], "-t") == 0)
            {
                sleep_time = std::stoull(argv[i+1]);
                if (sleep_time < 1)
                {
                    std::cout << "Error: the wait time must be greater than 0\n";
                    return 1;
                }
            }
            // the r option set the number of ping
            else if(strcmp(argv[i], "-r") == 0)
            {
                rounds = std::stoull(argv[i+1]);
                if (rounds < 1)
                {
                    std::cout << "Error: the number of rounds must be greater than 0\n";
                    return 1;
                }
            }
            // the n option set the number of threads
            else if(strcmp(argv[i], "-n") == 0)
            {
                nb_threads = std::stoull(argv[i+1]);
                if (nb_threads < 1)
                {
                    std::cout << "Error: the number of threads must be greater than 0\n";
                    return 1;
                }
            }
            // the h option print the help
            else if(strcmp(argv[i], "-h") == 0)
            {
                std::cout << "Usage: " << argv[0] << " [-t wait_time] [-r rounds] [-n nb_threads]\n";
                std::cout << "Default values: wait_time = 1s, rounds = 5, nb_threads = 2\n";
                return 0;
            }
            
        }
    }

    // create user defined arrays

    // queue to stock the ping times
    std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> ping_queue;

    // queue to stock the ping id
    std::queue <unsigned int> ping_id_queue;

    // mutex to protect the ping variable
    std::mutex ping_mutex;

    // threads array
    std::thread *threads = new std::thread[nb_threads];

    // queue to stock the ping times
    std::queue <long long unsigned int> *diff_queues = new std::queue <long long unsigned int> [nb_threads];

    // array to stock the actual ping times
    std::chrono::time_point<std::chrono::high_resolution_clock> *actual = new std::chrono::time_point<std::chrono::high_resolution_clock> [nb_threads];

    // array to stock the last ping times
    std::chrono::time_point<std::chrono::high_resolution_clock> *last = new std::chrono::time_point<std::chrono::high_resolution_clock> [nb_threads];

    // fill the array with the default value
    for(i = 0; i < nb_threads; i++)
    {
        actual[i] = std::chrono::time_point<std::chrono::high_resolution_clock>();
        last[i] = std::chrono::time_point<std::chrono::high_resolution_clock>();
    }

    // create the min, max, average, stdev and percent arrays
    long long unsigned int *min = new long long unsigned int[nb_threads];
    long long unsigned int *max = new long long unsigned int[nb_threads];
    long long unsigned int *avg = new long long unsigned int[nb_threads];
    double *stdev = new double[nb_threads];
    double *percent = new double[nb_threads];

    // fill the array with the default value
    for(i = 0; i < nb_threads; i++)
    {
        // fil min with the max value of an unsigned long long int
        min[i] = std::numeric_limits<long long unsigned int>::max();
        max[i] = std::numeric_limits<long long unsigned int>::min();
        avg[i] = 0;
        stdev[i] = 0;
        percent[i] = 0;
    }

    

    // start the threads
    for(i = 0; i < nb_threads; i++)
    {
        threads[i] = std::thread(ping, i, rounds, sleep_time, std::ref(ping_mutex), std::ref(ping_queue), std::ref(ping_id_queue));
    }



    // print the ping times
    std::cout << "--- Ping times ---\n";

    // get the pings and stock it in the pings array based on the ping id queue
    for(i = 0; i < nb_threads * (rounds + 1); i++)
    {
        //wait for a ping
        while(ping_queue.empty() && ping_id_queue.empty())
        {
            // wait for 1 ns
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }

        // get the ping
        // remove the ping from the queue
        ping_mutex.lock();
        id = ping_id_queue.front();
        actual[id] = ping_queue.front();
        ping_queue.pop();
        ping_id_queue.pop();
        ping_mutex.unlock();
            
        // print the ping time
        if(last[id] != std::chrono::time_point<std::chrono::high_resolution_clock>() && actual[id] != std::chrono::time_point<std::chrono::high_resolution_clock>())
        {
            // calculate the ping time
            diff = std::chrono::duration_cast<std::chrono::nanoseconds>(actual[id] - last[id]).count();
            diff -= sleep_time;

            std::cout << "Ping " << id << " latency : " << diff << " ns\n";

            // add the ping to the ping time array
            diff_queues[id].push(diff);
        }



        // update the last ping time
        last[id] = actual[id];
    }

    //for each thread
    for(i = 0; i < nb_threads; i++)
    {
        // calculate the min, max and average ping time
        avg[i] = 0;
         while (!diff_queues[i].empty()){
            // get the ping time
            diff = diff_queues[i].front();
            diff_queues[i].pop();

            // calculate the min, max and average ping time 
            if(diff < min[i])
            {
                min[i] = diff;
            }
            if(diff > max[i])
            {
                max[i] = diff;
            }
            avg[i] += diff;

            // fill the standard deviation array
            stdev[i] += static_cast<double>(diff) * static_cast<double>(diff);

        }
        // calculate the average over the rounds
        avg[i] /= rounds;

        // calculate the stdev
        stdev[i] = sqrt(stdev[i] / static_cast<double>(rounds));


        // calculate max in percent
        percent[i] = (static_cast<double>(max[i]) * 100.0) / static_cast<double>(sleep_time);

        // print the min, max and average ping time
        std::cout << "\n--- Thread " << i << " statistics ---\n";
        std::cout << "Min / max / avg / stdev (ns)\n";
        std::cout << min[i] << " / " << max[i] << " / " << avg[i] << " / " << stdev[i] << "\n";
        std::cout << std::fixed;
        std::cout << std::setprecision(6);
        std::cout << "Max in percent : " << percent[i] << "%\n";     
    }
    // join the threads
    for(i = 0; i < nb_threads; i++)
    {
        threads[i].join();
    }

    // delete the arrays
    delete[] threads;
    delete[] diff_queues;
    delete[] actual;
    delete[] last;
    delete[] min;
    delete[] max;
    delete[] avg;
    delete[] stdev;
    delete[] percent;
    
    return 0;
}

void ping(unsigned int tid, unsigned int rounds, long long unsigned int sleep_time, std::mutex &ping_mutex, std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> &ping_queue, std::queue <unsigned int> &ping_id_queue)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    unsigned int i =0;
    // get the start time
    start = std::chrono::high_resolution_clock::now();

    // add the first ping to the pingtime array
    ping_mutex.lock();
    ping_id_queue.push(tid);
    ping_queue.push(start);
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (i = 0; i < rounds; i++)
    {
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        do {

            end = std::chrono::high_resolution_clock::now();

        } while (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() < sleep_time);

        // add the ping to the ping time array
        ping_mutex.lock();
        ping_id_queue.push(tid);
        ping_queue.push(end);
        ping_mutex.unlock();
    }
}