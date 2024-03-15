#include <evl/clock.h>
#include <evl/thread.h>
#include <evl/timer.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>

#define TIME_INTERVAL_NANO 1000000

sigset_t start_periodic_timer() {

  // create timer
  tmdf = evl_new_timer(EVL_CLOCK_MONOTONIC)

      // read current time
      struct timespec now;
  evl_read_clock(EVL_CLOCK_MONOTONIC, &now);

  // set timerframe for 1ms
  struct itimerspec its;
  its.it_interval.tv_sec = 0;
  its.it_interval.tv_nsec = TIME_INTERVAL_NANO;

  its.it_value.tv_nsec = now.nsec + TIME_INTERVAL_NANO;
  its.it_value.tv_sec = now.sec

                        if (its.it_value.tv_nsec >= 1000000000) {
    its.it_value.tv_sec++;
    its.it_nsec -= 1000000000;
  }

  // set/start timer
  evl_set_timer(tmdf, its, nullptr)

      return tmfd;
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

  /* Attach the current thread to the EVL core. */
  int efd;
  efd = evl_attach_self("/real_time-%d", getpid());

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
  __u64 ticks;

  // start the timer
  int tmfd = start_periodic_timer();

  for (int index = 0; index < 1000; index++) {

    // get time before the math operation starts
    evl_read_clock(EVL_CLOCK_MONOTONIC, &start_time);
    some_random_computation();
    evl_read_clock(EVL_CLOCK_MONOTONIC, &before_sleep_time);

    // calculate and store the time taken by the math computation to be executed
    long long execution_time_ns =
        (before_sleep_time.tv_sec - start_time.tv_sec) * 1000000000 +
        (before_sleep_time.tv_nsec - start_time.tv_nsec);
    execution_time[index] = execution_time_ns;

    // sleep until timer is fired
    /* Wait for the next tick to be notified. */
    ret = oob_read(tmfd, &ticks, sizeof(ticks));

    // calculate the sample time
    evl_read_clock(EVL_CLOCK_MONOTONIC, &after_sleep_time);
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

  // detach the thread from evlcore
  evl_detach_thread();

  return nullptr;
}

int main() {
  // spawn a thread for the period task
  pthread_t thread;
  pthread_create(&thread, nullptr, periodic_task, nullptr);
  pthread_join(thread, nullptr);
  return 0;
}
