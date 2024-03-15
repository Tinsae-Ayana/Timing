#include <evl/clock.h>
#include <evl/thread.h>
#include <evl/timer.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//#include <evl/thread.h>
#define NSEC_PER_MSEC 1000000
#define NSEC_PER_SEC 1000000000

#define CYCLES 5000

void timespec_add_ns(struct timespec *ts, long ns) {
  ts->tv_nsec += ns;
  while (ts->tv_nsec >= NSEC_PER_SEC) {
    ts->tv_nsec -= NSEC_PER_SEC;
    ts->tv_sec++;
  }
}

void *periodicThread(void *arg) {
  struct itimerspec value;
  int tfd, tmfd, ret;
  __u64 ticks;
  struct timespec now, before, after;

  long long timeArraybefore[CYCLES];
  long long timeArrayafter[CYCLES];
  long long Intervalactual[CYCLES - 1];

  // mount the thread
  ret = evl_attach_self("my RT work thread");

  // debug mount
  if (ret < 0) {
    fprintf(stderr, "Failed to attach to EVL core: %s\n", strerror(-ret));
    return NULL;
  }
  // create the timer
  tmfd = evl_new_timer(EVL_CLOCK_MONOTONIC);
  // debug create
  if (tmfd < 0) {
    fprintf(stderr, "Failed to create EVL timer: %s\n", strerror(-tmfd));
    return NULL;
  }
  // set the timer
  evl_read_clock(EVL_CLOCK_MONOTONIC, &now);
  timespec_add_ns(
      &now,
      NSEC_PER_MSEC); // Add 1 ms to the current time,first trigger after 1ms
  value.it_value = now;
  value.it_interval.tv_sec = 0;
  value.it_interval.tv_nsec = NSEC_PER_MSEC; // Interval 1 ms
  ret = evl_set_timer(tmfd, &value, NULL);
  // debug test
  if (ret) {
    fprintf(stderr, "Failed to set EVL timer: %s\n", strerror(-ret));
    return NULL;
  }

  for (size_t i = 0; i < CYCLES; i++) {
    // block until the interval
    ret = oob_read(tmfd, &ticks, sizeof(ticks));
    // check
    if (ret < 0) {
      fprintf(stderr, "Failed to wait on EVL timer: %s\n", strerror(-ret));
      break;
    }
    // Checking whether missing trigger
    if (ticks > 1) {
      fprintf(stderr, "Timer overrun! %lld ticks late\n", ticks - 1);
    }
    // read clock-before
    evl_read_clock(EVL_CLOCK_MONOTONIC, &before);
    long long elapsed_ms1 = (before.tv_sec) * 1000000000 + (before.tv_nsec);
    timeArraybefore[i] = elapsed_ms1;

    // computational load
    volatile double dummy = 0.0;
    for (int j = 0; j < 250000; ++j) {
      dummy += j * 0.1;
    }
    // read clock-after
    evl_read_clock(EVL_CLOCK_MONOTONIC, &after);
    // long long elapsed_ns = (after.tv_sec - before.tv_sec) * NSEC_PER_SEC +
    // (after.tv_nsec - before.tv_nsec); printf("good - Computation took %lld
    // ns\n", elapsed_ns);
  }

  // Write result to file
  FILE *file = fopen("0314_EVL_Ass2_2.txt", "w");
  if (file == NULL) {
    perror("fopen");
    return NULL;
  }
  for (size_t i = 0; i < CYCLES - 1; i++) {
    Intervalactual[i] = timeArraybefore[i + 1] - timeArraybefore[i];
    fprintf(file, " nterval should be:%d ns, Actual Interval:%lld ns\n",
            1000000, Intervalactual[i]);
  }
  fclose(file);

  // evl_close_timer(tmfd);
  evl_detach_self();
  return NULL;
}

int main() {
  pthread_t threadId;

  if (pthread_create(&threadId, NULL, periodicThread, NULL) != 0) {
    perror("pthread_create failed");
    return EXIT_FAILURE;
  }

  pthread_join(threadId, NULL);
  return EXIT_SUCCESS;
}