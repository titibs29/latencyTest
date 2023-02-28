# latencyTest
---

This repos is made to test the latency of a computer.

## source code
--- 

the source code of the repos contains multiples files, listed over here

### sleeptest.cpp

this file use the function Sleep() (uSleep() on linux) to determine the latency between the wake up time asked and the real wake up time in nanosecond.

### nanosleepTest.cpp

same as sleepTest.cpp, except that it uses the function nanoSleep(), supposed to be more reactive.

### waitTest.cpp

this code is based on a busy-wait loop, it keep cycling until the time asked is over.

### mutlithreading.cpp

create a ping thread, then wait for the ping sent by the thread, based on a busy-wait loop.

### pluriping.cpp

can create multiple thread to send multiple ping to test consistency of hardware over multiple thread

## executing on windows
---

the executable files can be processed through g++, from the mwing64 suite, guide to install it is online

the compilation command is as follow :

´g++ <filename>.cpp -o <output-file>.exe -O2´

- filename.cpp : the name of the file you want to compile and execute
- output-file.exe : the name of the executable file
- -O2 : tell to the compiler to make the code fast !


## executing on linux
---

the gnu compiler can be used to compile and execute the code using the following command line :

´g++ <filename>.cpp -o <output-file>.exe -O2 -pthread´

- filename.cpp : the name of the file you want to compile and execute
- output-file.exe : the name of the executable file
- -O2 : tell to the compiler to make the code fast !
- -pthread : tell the compiler to use the pthread lib, only needed on the multithreading.cpp and pluriping.cpp

## arguments
---

once the executable ready, multiple arguments can be placed after it to customize the behavior as you wish:

- -h : show the help menu

- -t : define the wanted sleep time in nanosecond (1 second = 1 000 000 000 nanoseconds), default = 1 sec

- -r : indicates the number of rounds to go, default = 5 rounds

- -n : defines the number of threads to use, only in pluriping.cpp, default = 2