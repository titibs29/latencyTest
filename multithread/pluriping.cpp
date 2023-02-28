#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>
#include <cstring>
#include <iomanip>

void ping(int tid, int rounds, long long int sleep_time, std::mutex &ping_mutex, std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> &ping_queue, std::queue <int> &ping_id_queue);


int main(int argc, char *argv[])
{

    //define variables
    int id = 0;
    long long int diff = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

    
    int rounds = 5;
    long long int sleep_time = 1000000000; // 1 second
    int nb_threads = 2;



    // use the -t option to set the wait time, -r to set the number of ping, -n to set the number of threads
    if(argc > 1)
    {
        for(int i = 1; i < argc; i++)
        {
            if(strcmp(argv[i], "-t") == 0)
            {
                sleep_time = std::stoll(argv[i+1]);
            }
            else if(strcmp(argv[i], "-r") == 0)
            {
                rounds = std::stoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-n") == 0)
            {
                nb_threads = std::stoi(argv[i+1]);
            }
            else if(strcmp(argv[i], "-h") == 0)
            {
                std::cout << "Usage: " << argv[0] << " [-t wait_time] [-r rounds] [-n nb_threads]\n";
                std::cout << "Default values: wait_time = 1s, rounds = 5, nb_threads = 2\n";
                return 0;
            }
        }
    }

    // create the ping queue array
    std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> ping_queue;

    //create the ping id queue
    std::queue <int> ping_id_queue;

    // mutex to protect the ping variable
    std::mutex ping_mutex;

    // create the threads
    std::thread *threads = new std::thread[nb_threads];

    // create queue to stock the ping times
    std::queue <long long int> *diff_queues = new std::queue <long long int> [nb_threads];

    // create a array to stock last and actual ping times
    std::chrono::time_point<std::chrono::high_resolution_clock> *actual = new std::chrono::time_point<std::chrono::high_resolution_clock> [nb_threads];
    std::chrono::time_point<std::chrono::high_resolution_clock> *last = new std::chrono::time_point<std::chrono::high_resolution_clock> [nb_threads];

    // fill the array with the default value
    for(int i = 0; i < nb_threads; i++)
    {
        actual[i] = std::chrono::time_point<std::chrono::high_resolution_clock>();
        last[i] = std::chrono::time_point<std::chrono::high_resolution_clock>();
    }

    // create the min, max, average and median deviation arrays
    long long int *min = new long long int[nb_threads];
    long long int *max = new long long int[nb_threads];
    long long int *avg = new long long int[nb_threads];
    double *percent = new double[nb_threads];

    

    // start the threads

    for(int i = 0; i < nb_threads; i++)
    {
        threads[i] = std::thread(ping, i, rounds, sleep_time, std::ref(ping_mutex), std::ref(ping_queue), std::ref(ping_id_queue));
    }



    // print the ping times
    std::cout << "--- Ping times ---\n";

    // get the pings and stock it in the pings array based on the ping id queue
    for(int i = 0; i < nb_threads * (rounds + 1); i++)
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
    for(int i = 0; i < nb_threads; i++)
    {
        // calculate the min, max and average ping time
        min[i] = diff_queues[i].front();
        max[i] = 0;
        avg[i] = 0;
        for(int j = 0; j < rounds; j++)
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
        percent[i] = ((double)max[i] * 100.0) / (double)sleep_time;

        // print the min, max and average ping time
        std::cout << "\n--- Thread " << i << " statistics ---\n";
        std::cout << "Min / max / avg (ns)\n";
        std::cout << min[i] << " / " << max[i] << " / " << avg[i] << "\n";
        std::cout << std::fixed;
        std::cout << std::setprecision(6);
        std::cout << "Max in percent : " << percent[i] << "%\n";     
    }
    // join the threads
    for(int i = 0; i < nb_threads; i++)
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

void ping(int tid, int rounds, long long int sleep_time, std::mutex &ping_mutex, std::queue <std::chrono::time_point<std::chrono::high_resolution_clock>> &ping_queue, std::queue <int> &ping_id_queue)
{
    std::chrono::time_point<std::chrono::high_resolution_clock> start, end;
    // get the start time
    start = std::chrono::high_resolution_clock::now();

    // add the first ping to the pingtime array
    ping_mutex.lock();
    ping_id_queue.push(tid);
    ping_queue.push(start);
    ping_mutex.unlock();

    // loop until the number of ping is reached
    for (int i = 0; i < rounds; i++)
    {
        // get the start time
        start = std::chrono::high_resolution_clock::now();

        // wait for the ping to be done
        end = std::chrono::high_resolution_clock::now();

        while (std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() < sleep_time)
        {
            end = std::chrono::high_resolution_clock::now();
        }

        // add the ping to the ping time array
        ping_mutex.lock();
        ping_id_queue.push(tid);
        ping_queue.push(end);
        ping_mutex.unlock();
    }
}