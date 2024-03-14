#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#define TIME_INTERVAL_NANO 1000000

sigset_t start_periodic_timer() {

  // create timer
  timer_t timerid;
  struct sigevent sev;
  sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIGALRM;
  timer_create(CLOCK_MONOTONIC, &sev, &timerid);

  // set timerframe for 1ms
  struct itimerspec its;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1000000;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = 1000000;
  timer_settime(timerid, 0, &its, nullptr); // set/start timer

  // creating signal set that sigwait needs to wait for
  sigset_t signalset;
  sigemptyset(&signalset);
  sigaddset(&signalset, SIGALRM);

  // block SIGALRM before calling sigwait because it could be handled by default
  // handles asynchroneously and signwait wont' catch it, causes the program to
  // terminate

  sigprocmask(SIG_BLOCK, &signalset, NULL);

  return signalset;
}

void some_random_computation() {
  // calculate the sum from 1 to 10000
  // std::cout << "doing random computation" << std::endl;
  long long int sum = 0;
  for (long long int i = 10000; i >= 0; i--) {
    sum = sum + i;
  }
}

void *periodic_task(void *threadinput) {

  struct timespec start_time, before_sleep_time, after_sleep_time;

  // create a file to store the execution time
  FILE *file = fopen("data//execution_time1.txt", "a");
  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // creat a file to store the idle time
  FILE *file2 = fopen("data//idle_time1.txt", "a");
  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // creat a file to store the idle time
  FILE *file3 = fopen("data//sample_time1.txt", "a");
  if (file == NULL) {
    perror("Error opening file");
    exit(EXIT_FAILURE);
  }

  // array to store timing information, this is to avoid writing to file inside
  // the loop as per the assignment instruction
  long long int execution_time[1000];
  long long int idle_time[1000];
  long long int sample_time[1000];

  // start the timer
  sigset_t signalset = start_periodic_timer();

  for (int index = 0; index < 1000; index++) {

    // get time before the math operation starts
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    some_random_computation();
    clock_gettime(CLOCK_MONOTONIC, &before_sleep_time);

    // calculate and store the time taken by the math computation to be executed
    long long execution_time_ns =
        (before_sleep_time.tv_sec - start_time.tv_sec) * 1000000000 +
        (before_sleep_time.tv_nsec - start_time.tv_nsec);
    execution_time[index] = execution_time_ns;

    // sleep until signal is generated
    int sig;
    sigwait(&signalset, &sig);

    // calculate the sample time
    clock_gettime(CLOCK_MONOTONIC, &after_sleep_time);
    long long sample_time_ns =
        (after_sleep_time.tv_sec - start_time.tv_sec) * 1000000000 +
        (after_sleep_time.tv_nsec - start_time.tv_nsec);
    sample_time[index] = sample_time_ns;

    // calculate the idle time
    long long idle_time_ns =
        (after_sleep_time.tv_sec - before_sleep_time.tv_sec) * 1000000000 +
        (after_sleep_time.tv_nsec - before_sleep_time.tv_nsec);
    idle_time[index] = idle_time_ns;
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
  return nullptr;
}

int main() {

  // creating signal set that sigwait needs to wait for on the other thread but
  // in the main thread, it needs to blocked before it causes the program to
  // terminate
  sigset_t signalset;
  sigemptyset(&signalset);
  sigaddset(&signalset, SIGALRM);
  sigprocmask(SIG_BLOCK, &signalset, NULL);

  // spawn a thread for the period task
  pthread_t thread;
  pthread_create(&thread, nullptr, periodic_task, nullptr);
  pthread_join(thread, nullptr);
  return 0;
}
