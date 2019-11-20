#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#define VAL_NUM 1000000L
#define STACK_SIZE 1024

long stack[STACK_SIZE];
int stack_first = 0, stack_last = 0;

pthread_mutex_t stack_lock;
pthread_mutex_t popable;
pthread_mutex_t pushable;

pthread_t creator;
pthread_t consumer;

int push_block_count = 0;
int pop_block_count = 0;

void push(int val) {
    pthread_mutex_lock(&pushable);
    pthread_mutex_lock(&stack_lock);
    stack[stack_last] = val;
    stack_last = (stack_last + 1) % STACK_SIZE;
    if (stack_last != stack_first) {
        pthread_mutex_unlock(&pushable);
    }
    else {
        push_block_count++;
    }
    pthread_mutex_unlock(&popable);
    pthread_mutex_unlock(&stack_lock);
}

long pop() {
    long ret;
    pthread_mutex_lock(&popable);
    pthread_mutex_lock(&stack_lock);
    ret = stack[stack_first];
    stack_first = (stack_first + 1) % STACK_SIZE;
    if (stack_last != stack_first) {
        pthread_mutex_unlock(&popable);
    }
    else {
        pop_block_count++;
    }
    pthread_mutex_unlock(&pushable);
    pthread_mutex_unlock(&stack_lock);
    return ret;
}

void* creator_thread(void *arg) {
    for (long i = 0; i < VAL_NUM; i++) {
        push(i);
    }
    pthread_exit((void*)0);
}

void* consumer_thread(void *arg) {
    for (long i = 0; i < VAL_NUM; i++) {
        pop();
    }
    pthread_exit((void*)0);
}

int main() {
    pthread_mutex_init(&stack_lock, NULL);
    pthread_mutex_init(&pushable, NULL);
    pthread_mutex_init(&popable, NULL);

    pthread_create(&creator, NULL, creator_thread, NULL);
    pthread_create(&consumer, NULL, consumer_thread, NULL);
    pthread_join(creator, NULL);
    pthread_join(consumer, NULL);
    printf("Push Blocks: %d\n", push_block_count);
    printf("Pop Blocks: %d\n", pop_block_count);
    return 0;
}