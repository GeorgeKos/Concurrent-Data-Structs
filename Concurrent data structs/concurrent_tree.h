#ifndef _PROJECT1_TREE_H_
#define _PROJECT1_TREE_H_

struct TreeNode {
    int songID;
    TreeNode* lc;
    TreeNode* rc;
    TreeNode* parent; // pointer to the parent node
    pthread_mutex_t lock;
};

extern pthread_mutex_t tree_lock;
extern TreeNode* global_root;
extern int tree_size;
extern int total_keysum;
TreeNode* BST_search(int val, TreeNode *root,TreeNode* parent);
void BST_insert(int val, TreeNode* root,TreeNode* parent);
int BST_delete(TreeNode* to_be_deleted, TreeNode* root);
TreeNode *BST_inorder_successor(TreeNode *node);
TreeNode *BST_inorder_pred(TreeNode *node);
TreeNode* BST_createNode(int val,TreeNode* parent);
void DecrementTree(int val);
void BST_visitInorder(TreeNode* node);
#endif