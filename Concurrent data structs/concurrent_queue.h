#ifndef _PROJECT1_QUEUE_H_
#define _PROJECT1_QUEUE_H_

struct QueueNode {
    int value;
    QueueNode* next;
};

struct Queue {
    QueueNode* head;
    QueueNode* tail; 
    pthread_mutex_t headLock;
    pthread_mutex_t tailLock;
};

extern int queue_size;
extern int queue_keysum;
Queue* init_queue();
void enq(Queue* queue,int x);
int deq(Queue* queue);
#endif