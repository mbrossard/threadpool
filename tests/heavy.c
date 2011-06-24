#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"

#define THREAD 4
#define SIZE   1024
#define QUEUES 512

threadpool_t *pool[QUEUES];
int tasks[SIZE], left;
pthread_mutex_t lock;

void dummy_task(void *arg) {
    int *pi = (int *)arg;
    *pi = *pi + 1;
    if(*pi < QUEUES) {
        assert(threadpool_add(pool[*pi], &dummy_task, arg, 0) == 0);
    } else {
        pthread_mutex_lock(&lock);
        left--;
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char **argv)
{
    int i;

    pthread_mutex_init(&lock, NULL);

    for(i = 0; i < QUEUES; i++) {
        pool[i] = threadpool_create(THREAD, SIZE, 0);
    }

    left = SIZE - 1;
    for(i = 0; i < SIZE - 1; i++) {
        tasks[i] = 0;
        assert(threadpool_add(pool[0], &dummy_task, &(tasks[i]), 0) == 0);
    }
    while(left > 0) {
        usleep(10);
    }

    for(i = 0; i < QUEUES; i++) {
        threadpool_destroy(pool[i], 0);
    }

    pthread_mutex_destroy(&lock);

    return 0;
}
