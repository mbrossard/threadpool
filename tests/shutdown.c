#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>

#include "threadpool.h"

#define THREAD 4
#define SIZE   8192

threadpool_t *pool;
int left;
pthread_mutex_t lock;

int error;

void dummy_task(void *arg) {
    usleep(100);
    pthread_mutex_lock(&lock);
    left--;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    int i;

    pthread_mutex_init(&lock, NULL);

    /* Testing immediate shutdown */
    left = SIZE;
    pool = threadpool_create(THREAD, SIZE, 0);
    for(i = 0; i < SIZE; i++) {
        assert(threadpool_add(pool, &dummy_task, NULL, 0) == 0);
    }
    assert(threadpool_destroy(pool, 0) == 0);
    assert(left > 0);

    /* Testing graceful shutdown */
    left = SIZE;
    pool = threadpool_create(THREAD, SIZE, 0);
    for(i = 0; i < SIZE; i++) {
        assert(threadpool_add(pool, &dummy_task, NULL, 0) == 0);
    }
    assert(threadpool_destroy(pool, threadpool_graceful) == 0);
    assert(left == 0);

    pthread_mutex_destroy(&lock);

    return 0;
}
