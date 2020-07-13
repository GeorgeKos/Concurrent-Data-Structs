#include <pthread.h>
#include <iostream>
#include <vector>
#include "concurrent_tree.h"
#include "concurrent_queue.h"
#include "concurrent_list.h"
#include "Instrumentor.h"

int N = 0; // number of threads
int M = 0; // number of concurrent queues

std::vector<Queue*> queues;

pthread_barrier_t phase1_barrier;
pthread_barrier_t phase1_barrier_checks;
pthread_barrier_t phase2_barrier;
pthread_barrier_t phase2_barrier_checks;
pthread_barrier_t phase3_barrier;
pthread_barrier_t phase3_barrier_checks;

int check_1(){
    std::cout<< "total size check passed (expected:" << (N*N) <<" ,found:"<< tree_size <<")"<< std::endl;
    std::cout<< "total keysum check passed (expected:" << (N*N*(N-1)*(N+1)/2) << " ,found:"<< total_keysum <<")" << std::endl;
    if(tree_size != (N*N) || total_keysum != (N*N*(N-1)*(N+1)/2)) {
        return -1;
    }
    return 1;
}

int check_2(){
    std::cout<< "queues' total size check passed (expected:" << (N*N) <<" ,found:"<< queue_size <<")"<< std::endl;
    std::cout<< "total keysum check passed (expected:" << (N*N*(N-1)*(N+1)/2) << " ,found:"<< queue_keysum <<")" << std::endl;
    if(queue_size != (N*N) || queue_keysum != (N*N*(N-1)*(N+1)/2)) {
        return -1;
    }
    return 1;
}

int check_3(){
    std::cout<< "tree's size check passed (expected:" << ((N*N)/2) <<" ,found:"<< tree_size <<")"<< std::endl;
    std::cout<< "queues' total size check passed (expected:" << ((N*N)/2) <<" ,found:"<< queue_size <<")"<< std::endl;
    std::cout<< "list's total size check passed (expected:" << ((N*N)/2) <<" ,found:"<< shared_list_size <<")"<< std::endl;
    if(tree_size != ((N*N)/2) || queue_size != ((N*N)/2) || shared_list_size != ((N*N)/2)) {
        return -1;
    }
    return 1;
}

void* worker(void* arg) {
    //PROFILE_FUNCTION(); enable for benchmarking
    int thread_id = *((int*)&arg);
    int songID;
    int id_to_delete;
    TreeNode* tempnode;
    // ---------- PHASE 1 ----------
    for (int i = 0; i < N; i++) {
        songID = i * N + thread_id;
        BST_insert(songID,global_root,NULL);
    }
    pthread_barrier_wait(&phase1_barrier); // wait here till insertions are done

    if(thread_id == 0){ // thread with id 0 will make checks rest will wait
        if(check_1() == -1) {
            std::cout<<"Check 1 failed."<< std::endl;
            exit(-1);
        } else {
            std::cout<<"Check 1 Successful!" << std::endl;
            for (int i = 0; i < M; i++) {
                queues.emplace_back(init_queue()); // initialising the queues
            }
        }
    }
    pthread_barrier_wait(&phase1_barrier_checks); // waiting for thread 0 to finish checks 
    // now threads will become consumers 
    // ---------- PHASE 2 ----------
    for (int i = 0; i < N; i++) {
        songID = i * N + thread_id;
        enq(queues.at(thread_id % M),songID);
    }
    
    pthread_barrier_wait(&phase2_barrier);
    if(thread_id == 0){ // thread with id 0 will make checks rest will wait
        if(check_2() == -1) {
            std::cout<<"Check 2 failed."<< std::endl;
            exit(-1);
        } else {
            std::cout<<"Check 2 Successful!" << std::endl;
        }
    }
    pthread_barrier_wait(&phase2_barrier_checks);
    // ---------- PHASE 3 ----------
    for (int i = 0; i < N/2 ; i++) {
        id_to_delete = deq(queues.at(thread_id % M));
        tempnode = BST_search(id_to_delete,global_root,NULL);
        BST_delete(tempnode,global_root);
        LL_insert(id_to_delete);
    }
    pthread_barrier_wait(&phase3_barrier);
    if(thread_id == 0){ // thread with id 0 will make checks rest will wait
        if(check_3() == -1) {
            std::cout<<"Check 3 failed."<< std::endl;
            exit(-1);
        } else {
            std::cout<<"Check 3 Successful!" << std::endl;
        }
    }
    pthread_barrier_wait(&phase3_barrier_checks);
    return 0;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Need only 1 argument N , exiting program..." << std::endl;
        exit(-1);
    }
    N = atoi(argv[1]);
    M = N/2;

    pthread_barrier_init(&phase1_barrier,NULL,N);
    pthread_barrier_init(&phase1_barrier_checks,NULL,N);
    pthread_barrier_init(&phase2_barrier,NULL,N);
    pthread_barrier_init(&phase2_barrier_checks,NULL,N);
    pthread_barrier_init(&phase3_barrier,NULL,N);
    pthread_barrier_init(&phase3_barrier_checks,NULL,N);
    LL_init();
    pthread_t threads[N];
    for (int i = 0; i < N; i++) {
        pthread_create(&threads[i], NULL, worker, (void*)i);
    }

    for (int i = 0; i < N; i++) {
        pthread_join(threads[i],0);
    }
    
    pthread_barrier_destroy(&phase1_barrier);
    pthread_barrier_destroy(&phase1_barrier_checks);
    pthread_barrier_destroy(&phase2_barrier);
    pthread_barrier_destroy(&phase2_barrier_checks);
    pthread_barrier_destroy(&phase3_barrier);
    pthread_barrier_destroy(&phase3_barrier_checks);
    return 0;
}