// this is for  nano sleep method
#include <iostream>
#include <pthread.h>
#include <time.h>

#define INTERVAL_NS 1000000 // 1.0 ms in nanoseconds
using namespace std;

void some_random_computation() {
  // calculate the sum from 1 to 10000
  long long sum = 0;
  for (long long i = 10000; i >= 0; i--) {
    sum = sum + i;
  }
}

void *periodic_task(void *threadinput) {

  struct timespec currentStartTime, nextTime, lastStartTime, timeBeforeSleep,
      interval;
  bool isFirstIteration = true;
  interval.tv_sec = 0;
  interval.tv_nsec = INTERVAL_NS;

  // create a file to store the execution time (time taken by the math stuff)
  FILE *file = fopen("data2//execution_time1.txt", "a");
  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // creat a file to store the idle time (time the process sleeps)
  FILE *file2 = fopen("data2//idle_time1.txt", "a");
  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // create a file to store the sample time (time between conscutive loops)
  FILE *file3 = fopen("data2//sample_time1.txt", "a");
  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // array to store timing information, this is to avoid writing to file inside
  // the loop as per the assignment instruction
  long long int execution_time[1000];
  long long int idle_time[1000];
  long long int sample_time[1000];

  for (int i = 0; i < 1000; i++) {
    // record the current time at the beginning of each iteration
    clock_gettime(CLOCK_MONOTONIC, &currentStartTime);

    if (!isFirstIteration) {
      // calculate the time interval from the start of previous iteration to
      // current iteration
      long sample_time_ns =
          (currentStartTime.tv_sec - lastStartTime.tv_sec) * 1000000000L +
          (currentStartTime.tv_nsec - lastStartTime.tv_nsec);
      sample_time[i] = sample_time_ns;

      // calculate the idle time
      long long idle_time_ns =
          (currentStartTime.tv_sec - timeBeforeSleep.tv_sec) * 1000000000 +
          (currentStartTime.tv_nsec - timeBeforeSleep.tv_nsec);
      idle_time[i] = idle_time_ns;

    } else {
      isFirstIteration = false;
      nextTime = currentStartTime; // initialize it with current time. it is
                                   // updated downstairs
    }

    // save the start of current iteration to use it as last start in next
    // iteration
    lastStartTime = currentStartTime;

    // update next wait time. TODO:
    nextTime.tv_sec += (nextTime.tv_nsec + interval.tv_nsec) / 1000000000;
    nextTime.tv_nsec = (nextTime.tv_nsec + interval.tv_nsec) % 1000000000;

    // do some computation
    some_random_computation();

    // get absolute time before sleep
    clock_gettime(CLOCK_MONOTONIC, &timeBeforeSleep);

    // calculate and store the time taken by math computation to be executed
    long long execution_time_ns =
        (timeBeforeSleep.tv_sec - currentStartTime.tv_sec) * 1000000000 +
        (timeBeforeSleep.tv_nsec - currentStartTime.tv_nsec);
    execution_time[i] = execution_time_ns;

    // sleep the remaining idle time
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nextTime, NULL);
  }

  // save the data to file
  for (int i = 0; i < 100; i++) {
    fprintf(file, "%lld\n", execution_time[i]);
  }

  for (int i = 0; i < 100; i++) {
    fprintf(file2, "%lld\n", idle_time[i]);
  }

  for (int i = 0; i < 100; i++) {
    fprintf(file3, "%lld\n", sample_time[i]);
  }

  // close the files
  fclose(file);
  fclose(file2);
  fclose(file3);

  return NULL;
}

int main() {
  pthread_t periodic;
  pthread_create(&periodic, nullptr, periodic_task, nullptr);
  pthread_join(periodic, nullptr);
  return 0;
}
