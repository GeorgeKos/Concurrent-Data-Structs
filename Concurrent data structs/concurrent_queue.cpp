#include <iostream>
#include <assert.h>
#include <pthread.h>
#include "concurrent_queue.h"

int queue_size=0;
int queue_keysum=0;
pthread_mutex_t queue_inc_lock; // lock for the increments
pthread_mutex_t queue_dec_lock; // lock for the decrements

Queue* init_queue(){
    Queue* queue = NULL;
    QueueNode* queue_sentinel = NULL;

    queue_sentinel = new QueueNode();
    queue_sentinel->value = -1;
    queue_sentinel->next = NULL;

    queue = new Queue();
    queue->head  = queue_sentinel;
    queue->tail  = queue_sentinel;
    pthread_mutex_init(&queue->headLock, NULL);
    pthread_mutex_init(&queue->tailLock, NULL);
    return queue;
}

void enq(Queue* queue,int x){
    QueueNode* node = new QueueNode();
    node->value = x;
    node->next = NULL;

    pthread_mutex_lock(&queue->tailLock);
    queue->tail->next = node;
    queue->tail = node;
    pthread_mutex_lock(&queue_inc_lock);
    queue_size++;
    queue_keysum += node->value;
    pthread_mutex_unlock(&queue_inc_lock);
    pthread_mutex_unlock(&queue->tailLock);
}

int deq(Queue* queue){
    int result;
    pthread_mutex_lock(&queue->headLock);
    if(queue->head->next == NULL){
        result = -1;
    }else{
        result = queue->head->next->value;
        queue->head = queue->head->next;
        pthread_mutex_lock(&queue_dec_lock);
        queue_size--;
        queue_keysum -= result;
        pthread_mutex_unlock(&queue_dec_lock);
    }
    pthread_mutex_unlock(&queue->headLock);
    return result;
}