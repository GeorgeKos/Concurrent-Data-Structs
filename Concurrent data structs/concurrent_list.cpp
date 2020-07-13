#include <iostream>
#include <assert.h>
#include <pthread.h>
#include "concurrent_list.h"


s_list* shared_list = NULL;
int shared_list_size = 0;
pthread_mutex_t list_inc_lock;

void LL_init(){
    listNode* dummy_tail = new listNode();
    dummy_tail->next = NULL;
    dummy_tail->songID = -1;
    listNode* dummy_head = new listNode();
    dummy_head->next = dummy_tail;
    dummy_head->songID = -1;

    shared_list = new s_list();
    shared_list->head = dummy_head;
    shared_list->tail = dummy_tail;
    pthread_mutex_init(&shared_list->head->lock,NULL);
    pthread_mutex_init(&shared_list->tail->lock,NULL);  
}

bool LL_validate(listNode* pred, listNode* curr) {
    listNode* tmp = shared_list->head;
    while (tmp->songID <= pred->songID) {
        if (tmp == pred) {
            if (pred->next == curr) return true;
            else return false;
        }
        tmp = tmp->next;
    }
    return false;
}

bool LL_search(int key) {
    listNode* curr, *pred;
    int return_flag;
    bool result;
    while (true) {
        pred = shared_list->head; 
        curr = pred->next;
        while ((curr->next!= NULL) && curr->songID < key) {
            pred = curr; 
            curr = curr->next;
        }
        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);
        if (LL_validate(pred, curr) == true) {
            if (key == curr->songID) result = true;
            else result = false;
            return_flag = 1;
        }
        pthread_mutex_unlock(&pred->lock);
        pthread_mutex_unlock(&curr->lock);
        if (return_flag) return result; 
    }
}

bool LL_insert(int key) { // code for process p
    listNode* pred, *curr;
    bool result;
    bool return_flag = 0;
    while (true) {
        pred = shared_list->head; 
        curr = pred->next;
        while ((curr->next!= NULL) && curr->songID < key) {
            pred = curr;
            curr = curr->next;
        }
        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);
        if (LL_validate(pred, curr) == true) {
            if (key == curr->songID) {
                result = false; 
                return_flag = 1;
            } else {
                listNode* node = new listNode();
                node->next = curr;
                node->songID = key;
                pred->next = node;
                pthread_mutex_lock(&list_inc_lock);
                shared_list_size++; // inserting
                pthread_mutex_unlock(&list_inc_lock);
                result = true;
                return_flag = 1;
            }
        }
        pthread_mutex_unlock(&pred->lock);
        pthread_mutex_unlock(&curr->lock);
        if (return_flag) return result;
    }
}

bool LL_delete(int key) {
    listNode* pred, *curr;
    bool result; 
    bool return_flag = 0;
    while (true) {
        pred = shared_list->head;
        curr = pred->next;
        while ((curr->next!= NULL) && curr->songID < key) {
            pred = curr;
            curr = curr->next;
        }
        pthread_mutex_lock(&pred->lock);
        pthread_mutex_lock(&curr->lock);
        if (LL_validate(pred, curr)) {
            if (key == curr->songID) {
                pred->next = curr->next;
                pthread_mutex_lock(&list_inc_lock);
                shared_list_size--; // removing
                pthread_mutex_unlock(&list_inc_lock);
                result = true;
            }
        } else {
            result = false;
        }
        return_flag = 1;
    }
    pthread_mutex_unlock(&pred->lock);
    pthread_mutex_unlock(&curr->lock);
    if (return_flag == 1) return result;
}