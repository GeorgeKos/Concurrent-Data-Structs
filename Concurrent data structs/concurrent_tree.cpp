#include <iostream>
#include <assert.h>
#include <pthread.h>
#include <assert.h>
#include "concurrent_tree.h"

pthread_mutex_t tree_lock;
pthread_mutex_t tree_inc_lock;
pthread_mutex_t tree_dec_lock;
TreeNode* global_root = NULL;
int tree_size = 0;
int total_keysum = 0;


TreeNode* BST_search(int val, TreeNode *root, TreeNode *parent) {
    if (parent == NULL) {  //I am at the root
        pthread_mutex_lock(&tree_lock);
        if (global_root == NULL) {
            std::cout<<"Cant find node with value" <<val<<std::endl;
            pthread_mutex_unlock(&tree_lock);
            return NULL;
        }
        pthread_mutex_lock(&global_root->lock);
        root = global_root;
        pthread_mutex_unlock(&tree_lock);
    }

    if (val < root->songID) {
        if (root->lc == NULL) {
            std::cout<<"Cant find node with value" <<val<<std::endl;
            pthread_mutex_unlock(&root->lock);
            return NULL;
        } else {
            pthread_mutex_lock(&root->lc->lock);
            pthread_mutex_unlock(&root->lock);
            return BST_search(val, root->lc, root);
        }
    } else if (val > root->songID) {
        if (root->rc == NULL) {
            std::cout<<"Cant find node with value" <<val<<std::endl;
            pthread_mutex_unlock(&root->lock);
            return NULL;
        } else {
            pthread_mutex_lock(&root->rc->lock);
            pthread_mutex_unlock(&root->lock);
            return BST_search(val, root->rc, root);
        }
    } else {
        pthread_mutex_unlock(&root->lock);
        return root;
    }
    return root;
}

void BST_insert(int val, TreeNode *root, TreeNode *parent) {
    if (parent == NULL) {  //I am at the root
        pthread_mutex_lock(&tree_lock);
        if (global_root == NULL) {
            global_root = BST_createNode(val, parent);
            pthread_mutex_unlock(&tree_lock);
            return;
        }
        pthread_mutex_lock(&global_root->lock);
        root = global_root;
        pthread_mutex_unlock(&tree_lock);
    }

    if (val < root->songID) {
        if (root->lc == NULL) {
            root->lc = BST_createNode(val, root);
            pthread_mutex_unlock(&root->lock);
        } else {
            pthread_mutex_lock(&root->lc->lock);
            pthread_mutex_unlock(&root->lock);
            BST_insert(val, root->lc, root);
        }
    } else if (val > root->songID) {
        if (root->rc == NULL) {
            root->rc = BST_createNode(val, root);
            pthread_mutex_unlock(&root->lock);
        } else {
            pthread_mutex_lock(&root->rc->lock);
            pthread_mutex_unlock(&root->lock);
            BST_insert(val, root->rc, root);
        }
    } else {
        printf("Duplicates not allowed");
        assert(0);
    }
}

TreeNode *BST_createNode(int val, TreeNode *parent) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));

    if (node == NULL) {
        std::cout<<"Failed to allocate memory";
        return node;
    }

    node->songID = val;
    node->lc = NULL;
    node->rc = NULL;
    node->parent = parent;
    pthread_mutex_init(&node->lock, NULL);
    pthread_mutex_lock(&tree_inc_lock); // acquiring lock for increments;
    tree_size++;
    total_keysum+= node->songID;
    pthread_mutex_unlock(&tree_inc_lock);
    return node;
}

int BST_delete(TreeNode* to_be_deleted,TreeNode* root){
    TreeNode *parent;
    TreeNode *pred, *pred_parent;
    TreeNode *successor, *successor_parent;

    pthread_mutex_lock(&tree_lock);
    if (global_root == NULL) {
        std::cout<< "Tree is empty"<<std::endl;
        pthread_mutex_unlock(&tree_lock);
        return -2;
    }

    //pthread_mutex_lock(&root->lock);
    if ( (to_be_deleted->songID == root->songID) && root->lc == NULL && root->rc == NULL ) { // if we are at root and it is the only node
        
        DecrementTree(root->songID);
        free(root);
        global_root = NULL;
        pthread_mutex_unlock(&tree_lock);
        return 1;
    }

    parent = to_be_deleted->parent;

    if (to_be_deleted->lc == NULL && to_be_deleted->rc == NULL &&
        parent == NULL) {
        DecrementTree(to_be_deleted->songID);
        free(to_be_deleted);
        global_root = NULL;
        pthread_mutex_unlock(&tree_lock);
        return 1;
    }
    pthread_mutex_unlock(&tree_lock);

    // Leaf to be deleted
    if (to_be_deleted->lc == NULL && to_be_deleted->rc == NULL) {
        if (to_be_deleted->songID < parent->songID) {
            DecrementTree(to_be_deleted->songID);
            free(to_be_deleted);
            parent->lc = NULL;
            pthread_mutex_unlock(&parent->lock);
            return 1;
        } else {
            // node to be deleted is the right child of its parent
            DecrementTree(to_be_deleted->songID);
            free(to_be_deleted);
            parent->rc = NULL;
            pthread_mutex_unlock(&parent->lock);
            return 1;
        }
    }

    if (parent != NULL) {
        pthread_mutex_unlock(&parent->lock);
    }

    if (to_be_deleted->rc != NULL) {
        // finding successor
        pthread_mutex_lock(&to_be_deleted->rc->lock);
        if (to_be_deleted->rc->lc == NULL) {
            successor = to_be_deleted->rc;
            if (successor->rc != NULL) {
                pthread_mutex_lock(&successor->rc->lock);
                to_be_deleted->songID = successor->songID;
                to_be_deleted->rc = successor->rc;
                successor->rc->parent = to_be_deleted;
                pthread_mutex_unlock(&successor->rc->lock);
            } else {
                to_be_deleted->songID = successor->songID;
                to_be_deleted->rc = NULL;
            }
            //pthread_mutex_unlock(&successor->lock);
            DecrementTree(successor->songID);
            free(successor);
            pthread_mutex_unlock(&to_be_deleted->lock);
            return 1;
        }
        successor = BST_inorder_successor(to_be_deleted);
        successor_parent = successor->parent;

        if (successor->rc != NULL) {
            pthread_mutex_lock(&successor->rc->lock);
            successor_parent->lc = successor->rc;
            successor->rc->parent = successor_parent;
            to_be_deleted->songID = successor->songID;
            pthread_mutex_unlock(&successor->rc->lock);
        } else {
            to_be_deleted->songID = successor->songID;
            successor_parent->lc = NULL;
        }
        DecrementTree(successor->songID);
        free(successor);
        pthread_mutex_unlock(&successor_parent->lock);
        pthread_mutex_unlock(&to_be_deleted->lock);
        
        return 1;
    }
    if (to_be_deleted->lc != NULL) {
        pthread_mutex_lock(&to_be_deleted->lc->lock);
        if (to_be_deleted->lc->rc == NULL) {
            pred = to_be_deleted->lc;
            if (pred->lc != NULL) {
                pthread_mutex_lock(&pred->lc->lock);
                to_be_deleted->songID = pred->songID;
                to_be_deleted->lc = pred->lc;
                pred->lc->parent = to_be_deleted;
                pthread_mutex_unlock(&pred->lc->lock);
            } else {
                to_be_deleted->songID = pred->songID;
                to_be_deleted->lc = NULL;
            }
            DecrementTree(pred->songID);
            free(pred);
            pthread_mutex_unlock(&to_be_deleted->lock);
            
            return 1;
        }
        pred = BST_inorder_pred(to_be_deleted);
        pred_parent = pred->parent;

        if (pred->lc != NULL) {
            pthread_mutex_lock(&pred->lc->lock);
            pred_parent->rc = pred->lc;
            pred->lc->parent = pred_parent;
            to_be_deleted->songID = pred->songID;
            pthread_mutex_unlock(&pred->lc->lock);
        } else {
            to_be_deleted->songID = pred->songID;
            pred_parent->rc = NULL;
        }
        DecrementTree(pred->songID);
        free(pred);
        pthread_mutex_unlock(&pred_parent->lock);
        pthread_mutex_unlock(&to_be_deleted->lock);
        
        return 1;
    }
    return -1;
}

TreeNode *BST_inorder_successor(TreeNode *node) {
    TreeNode *parent, *successor;

    parent = node->rc;
    successor = parent->lc;

    pthread_mutex_lock(&successor->lock);
    while (successor->lc != NULL) {
        successor = successor->lc;
        pthread_mutex_unlock(&parent->lock);
        pthread_mutex_lock(&successor->lock);
        parent = successor->parent;
    }
    return successor;
}

TreeNode *BST_inorder_pred(TreeNode *node) {
    TreeNode *parent, *pred;

    parent = node->lc;
    pred = parent->rc;

    pthread_mutex_lock(&pred->lock);
    while (pred->rc != NULL) {
        pred = pred->rc;
        pthread_mutex_unlock(&parent->lock);
        pthread_mutex_lock(&pred->lock);
        parent = pred->parent;
    }
    return pred;
}

void DecrementTree(int val){
    pthread_mutex_lock(&tree_dec_lock);
    tree_size--;
    total_keysum -= val;
    pthread_mutex_unlock(&tree_dec_lock);
}
// prints the tree recursively with InOrder
void BST_visitInorder(TreeNode* node) 
{ 
    if (node == NULL) {
        return; 
    }
    BST_visitInorder(node->lc); 
    //std::cout << node->songID << " " <<std::endl; 
    BST_visitInorder(node->rc);
} 
