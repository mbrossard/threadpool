#define THREAD 32
#define QUEUE  256

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "threadpool.h"

int tasks = 0, done = 0;
pthread_mutex_t lock;

void dummy_task(void *arg) {
    sleep(1);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char **argv)
{
    threadpool_t *pool;

    pthread_mutex_init(&lock, NULL);

    pool = threadpool_create(THREAD, QUEUE, 0);
    fprintf(stderr, "Pool started with %d threads and "
            "queue size of %d\n", THREAD, QUEUE);

    while(threadpool_add(pool, &dummy_task, NULL, 0) == 0) {
        pthread_mutex_lock(&lock);
        tasks++;
        pthread_mutex_unlock(&lock);
    }
    fprintf(stderr, "Added %d tasks\n", tasks);

    while(tasks / 2 > done) {
        sleep(1);
    }
    fprintf(stderr, "Did %d tasks before shutdown\n", done);
    threadpool_destroy(pool, 0);
    fprintf(stderr, "Did %d tasks\n", done);

    return 0;
}
