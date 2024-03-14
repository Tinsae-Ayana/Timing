#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <evl/thread.h>
#include <evl/timer.h>
#include <evl/clock.h>
#include <evl/proxy.h>

#define TIME_INTERVAL_NANO 1000000

void timespec_add_ns(struct timespec *__restrict r,
	     		     const struct timespec *__restrict t,
			         long ns) {
    long s, rem;

    s = ns / 1000000000;
    rem = ns - s * 1000000000;
    r->tv_sec = t->tv_sec + s;
    r->tv_nsec = t->tv_nsec + rem;
    if (r->tv_nsec >= 1000000000) {
            r->tv_sec++;
            r->tv_nsec -= 1000000000;
    }
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
    struct itimerspec value, ovalue;
    int tfd, tmfd, ret, n = 0;
    struct timespec now, currentStartTime, lastStartTime, timeBeforeSleep;
    bool isFirstIteration = true;
    __u64 ticks;

    // attach to the core
    tfd = evl_attach_self("periodic-timer:%d", getpid());
    check_this_fd(tfd);

    // creat a timer
    tmfd = evl_new_timer(EVL_CLOCK_MONOTONIC);
    check_this_fd(tmfd);

    // set timer for 1ms
    ret = evl_read_clock(EVL_CLOCK_MONOTONIC, &now); 
    check_this_status(ret); 
    timespec_add_ns(&value.it_value, &now, TIME_INTERVAL_NANO);
    value.it_interval.tv_sec = 0; 
    value.it_interval.tv_nsec = TIME_INTERVAL_NANO;
    ret = evl_set_timer(tmfd, &value, &ovalue); 
    check_this_status(ret);

    long long int execution_time[1000];
    long long int idle_time[1000];
    long long int sample_time[1000];

    for (int index = 0; index < 1000; index++) {
        // wait for the next tick to be notified
        ret = oob_read(tmfd, &ticks, sizeof(ticks));
        check_this_status(ret);
        if (ticks > 1) {
            fprintf(stderr, "timer overrun! %lld ticks late\n", ticks - 1);
        }

        // record the current time at the beginning of each iteration
        evl_read_clock(EVL_CLOCK_MONOTONIC, &currentStartTime);

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
        }

        // save the start of current iteration to use it as last start in next
        lastStartTime = currentStartTime;

        // do some computation
        some_random_computation();

        // get absolute time before sleep
        evl_read_clock(EVL_CLOCK_MONOTONIC, &timeBeforeSleep);

        // calculate and store the time taken by math computation to be executed
        long long execution_time_ns =
            (timeBeforeSleep.tv_sec - currentStartTime.tv_sec) * 1000000000 +
            (timeBeforeSleep.tv_nsec - currentStartTime.tv_nsec);
        execution_time[i] = execution_time_ns;
    }

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

    // disable the timer
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;
    ret = evl_set_timer(tmfd, &value, NULL);
    check_this_status(ret);

    evl_detach_self();
    return nullptr;
}

int main() {
    pthread_t threadId;

    if (pthread_create(&threadId, nullptr, periodicThread, nullptr) != 0) {
        perror("pthread_create failed");
        return EXIT_FAILURE;
    }

    pthread_join(threadId, nullptr);
    return 0;
}