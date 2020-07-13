#ifndef _PROJECT1_LIST_H_
#define _PROJECT1_LIST_H_

struct listNode {
    int songID; // the key
    struct listNode* next;
    pthread_mutex_t lock;
}; 

struct s_list {
    listNode* head;
    listNode* tail;
};

extern s_list* shared_list;
extern int shared_list_size;
void LL_init();
bool LL_validate(listNode* pred, listNode* curr);
bool LL_search(int key);
bool LL_insert(int key);
bool LL_delete(int key);

#endif