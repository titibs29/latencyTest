#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>
#include <cstring>
#include <iomanip>

void ping(unsigned int tid, unsigned int rounds, long long unsigned int sleep_time, std::mutex &ping_mutex, std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> &ping_queue, std::queue <unsigned int> &ping_id_queue);


int main(int argc, char *argv[])
{

    //define variables
    unsigned int id = 0, i = 0, j = 0;
    long long unsigned int diff = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    
    long long unsigned int rounds = 5;
    long long unsigned int sleep_time = 1000000000; // 1 second
    long long unsigned int nb_threads = 1;



    // use the -t option to set the wait time, -r to set the number of ping, -n to set the number of threads
    if(argc > 1)
    {
        for(i = 1; i < static_cast<unsigned int>(argc); i++)
        {
            if(strcmp(argv[i], "-t") == 0)
            {
                sleep_time = std::stoull(argv[i+1]);
                if (sleep_time < 1)
                {
                    std::cout << "Error: the wait time must be greater than 0\n";
                    return 1;
                }
            }
            else if(strcmp(argv[i], "-r") == 0)
            {
                rounds = std::stoull(argv[i+1]);
                if (rounds < 1)
                {
                    std::cout << "Error: the number of rounds must be greater than 0\n";
                    return 1;
                }
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                std::cout << "Usage: " << argv[0] << " [-t wait_time] [-r rounds]\n";
                std::cout << "Default values: wait_time = 1s, rounds = 5\n";
                return 0;
            }
            
        }
    }

    // create the ping queue array
    std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> ping_queue;

    //create the ping id queue
    std::queue <unsigned int> ping_id_queue;

    // mutex to protect the ping variable
    std::mutex ping_mutex;

    // create the threads
    std::thread *threads = new std::thread[nb_threads];

    // create queue to stock the ping times
    std::queue <long long unsigned int> *diff_queues = new std::queue <long long unsigned int> [nb_threads];

    // create a array to stock last and actual ping times
    std::chrono::time_point<std::chrono::high_resolution_clock> *actual = new std::chrono::time_point<std::chrono::high_resolution_clock> [nb_threads];
    std::chrono::time_point<std::chrono::high_resolution_clock> *last = new std::chrono::time_point<std::chrono::high_resolution_clock> [nb_threads];

    // fill the array with the default value
    for(i = 0; i < nb_threads; i++)
    {
        actual[i] = std::chrono::time_point<std::chrono::high_resolution_clock>();
        last[i] = std::chrono::time_point<std::chrono::high_resolution_clock>();
    }

    // create the min, max, average and median deviation arrays
    long long unsigned int *min = new long long unsigned int[nb_threads];
    long long unsigned int *max = new long long unsigned int[nb_threads];
    long long unsigned int *avg = new long long unsigned int[nb_threads];
    double *percent = new double[nb_threads];

    

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
            std::cout << "";
        }
        // get the ping
        id = ping_id_queue.front();
        actual[id] = ping_queue.front();
        ping_mutex.lock();
        ping_queue.pop();
        ping_id_queue.pop();
        ping_mutex.unlock();

        // print the ping time
        if(last[id] != std::chrono::time_point<std::chrono::high_resolution_clock>() && actual[id] != std::chrono::time_point<std::chrono::high_resolution_clock>())
        {
            diff = static_cast<unsigned>(std::chrono::duration_cast<std::chrono::nanoseconds>(actual[id] - last[id]).count());
            diff -= sleep_time;
            std::cout << "Ping latency : " << diff << " ns\n";

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
        min[i] = diff_queues[i].front();
        max[i] = 0;
        avg[i] = 0;
        for(j = 0; j < rounds; j++)
        {
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


        }
        avg[i] /= rounds;

        // calculate max in percent
        percent[i] = (static_cast<double>(max[i]) * 100.0) / static_cast<double>(sleep_time);

        // print the min, max and average ping time
        std::cout << "\n--- Ping statistics ---\n";
        std::cout << "Min / max / avg (ns)\n";
        std::cout << min[i] << " / " << max[i] << " / " << avg[i] << "\n";
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
    for (i = 0; i < rounds+1; i++)
    {
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        do {

            end = std::chrono::high_resolution_clock::now();

        } while (static_cast<long long unsigned int>(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()) < sleep_time);

        // add the ping to the ping time array
        ping_mutex.lock();
        ping_id_queue.push(tid);
        ping_queue.push(end);
        ping_mutex.unlock();
    }
}