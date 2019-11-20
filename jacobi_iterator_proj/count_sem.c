#include <pthread.h>

#define THREAD_COUNT 8

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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#define MATRIX_ROWS 1024
#define MATRIX_COLUMNS 1024

const float epsilon = 0.001f;

float (*m)[MATRIX_COLUMNS];

typedef struct subtask subtask_t;
struct subtask {
    subtask_arg_t subtask_arg;
    subtask_ret_t subtask_ret;
};

typedef struct subtask_arg subtask_arg_t;
struct subtask_arg {
    unsigned row_start;
    unsigned row_end;
    unsigned col_start;
    unsigned col_end;
};

typedef struct subtask_ret subtask_ret_t;
struct subtask_ret {
    float delta_max;
};

float average(unsigned row, unsigned col) {
    float prev = m[row][col];
    m[row][col] = (m[row-1][col] + m[row][col-1] + m[row+1][col] + \
                   m[row][col+1])/4;
    return fabs(prev - m[row][col]);
}

void* jacobi_itterate_subtask(void* arg) {
    float delta;
    subtask_arg_t *subtask_arg = &(((subtask_t*)arg)->subtask_arg);
    subtask_ret_t *subtask_ret = &(((subtask_t*)arg)->subtask_ret);

    subtask_ret->delta_max = average(subtask_arg->row_start, \
                                     subtask_arg->col_start);
    for (unsigned row = subtask_arg->row_start + 1; row < subtask_arg->row_end; \
            row++) {
        for (unsigned col = subtask_arg->col_start + 1; col < subtask_arg->col_end; \
                col++) {
            delta = average(row, col);
            if (delta > subtask_ret->delta_max) {
                subtask_ret->delta_max = delta;
            }
        }
    }
    sem_sync_wait(&barrier);
    pthread_exit((void*)subtask_ret);
}

/**
 * Completes one itteration of the jacobi process
 * Returns: Largest delta between averages.
 */
float jacobi_itterate() {
    void* ret;
    pthread_t threads[THREAD_COUNT];
    subtask_t subtasks[THREAD_COUNT];
    float delta_max;
    
    /**
     * Initialize subtasks (partition sections of matrix)
     */

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&(threads[i]), NULL, jacobi_itterate_subtask, \
            (void*)(&subtasks[i]));
    }
    sem_sync_wait(&barrier);

    pthread_join(threads[0], &(subtasks[0].subtask_ret));
    delta_max = subtasks[0].subtask_ret.delta_max;
    for (int i = 1; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], &(subtasks[i].subtask_ret));
        if (subtasks[i].subtask_ret.delta_max > delta_max) {
            delta_max = subtasks[i].subtask_ret.delta_max;
        }
    }

    return delta_max;
}

int init_matrix(FILE *input) {
    int ret = 0;
    errno = 0;
    m = malloc(sizeof(float) * MATRIX_ROWS * MATRIX_COLUMNS);
    if (m == NULL) {
        return -1;
    }
    /**
     * If something aint float, sink the fucking boat.
     */
    char *line = getline
    strtof();
}

int main(int argc, char **args) {
    if (args[1] == NULL) {
        printf("No input given\n");
        return 1;
    }
    else {
        FILE *input = fopen(args[1], "r");
        if (input == NULL) {
            printf("Could not open file %s\n", args[1]);
            return 1;
        }
        else if (init_matrix(input) < 0) {

        }
        else {
            sem_sync_init(&barrier);
            float delta_max = jacobi_itterate();
            while (epsilon <= jacobi_itterate()) {
                delta_max = jacobi_itterate();
            }

            /**
             * Output file
             */
        }
    }
    

    return 0;
}