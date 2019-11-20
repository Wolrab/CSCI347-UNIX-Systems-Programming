#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define THREAD_COUNT 9

pthread_t threads[THREAD_COUNT];

typedef struct sem_sync sem_sync_t;
struct sem_sync {
    pthread_mutex_t mtx;
    pthread_cond_t threads_synced;
    int thread_count;
    int thread_count_synced;
};

sem_sync_t barrier;

void sem_sync_init(sem_sync_t *barrier) {
    pthread_mutex_init(&(barrier->mtx), NULL);
    pthread_cond_init(&(barrier->threads_synced), NULL);
    barrier->thread_count = 0;
    barrier->thread_count_synced = THREAD_COUNT + 1;
}

void sem_sync_wait(sem_sync_t *barrier) {
    pthread_mutex_lock(&(barrier->mtx));
    barrier->thread_count++;
    if (barrier->thread_count < barrier->thread_count_synced) {
        pthread_cond_wait(&(barrier->threads_synced), &(barrier->mtx));
    }
    else {
        barrier->thread_count = 0;
        pthread_cond_broadcast(&(barrier->threads_synced));
    }
    pthread_mutex_unlock(&(barrier->mtx));
}

void* do_stuff(void* arg) {
    printf("Thread %d\n", (int)arg);
    sem_sync_wait(&barrier);
    pthread_exit(arg);
}

int main(int argc, char **args) {
    void* ret;
    sem_sync_init(&barrier);

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&(threads[i]), NULL, do_stuff, (void*)i);
    }
    sem_sync_wait(&barrier);
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], &ret);
        printf("Got thread %d\n", (int)ret);
    }
    return 0;
}