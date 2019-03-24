/*
 * @Author: xiaoyi.luo 
 * @Date: 2019-03-24 15:25:23 
 * @Last Modified by: xiaoyi.luo
 * @Last Modified time: 2019-03-24 18:06:43
 */
#include <stdio.h>
#include <stdlib.h>
// 颜色定义
typedef enum Color{
    BLACK,
    RED
} Color;
// 节点定义
typedef struct TNode{
    struct TNode* left;
    struct TNode* right;
    struct TNode* parent;
    Color color;
    int value; // 可以采用 void* 来保存任意类型，不过这样的话，
               // 就需要提供一个 compare 的函数指针了，比较麻烦，
               // 这里就用 int，目的是理解数据结构本身
} TNode;

typedef struct RbTree{
    TNode * root;
} RbTree;

TNode* grand_parent(const TNode *n){
    if(n && n->parent)
        return n->parent->parent;
    else
        return NULL;
 }

TNode* uncle(const TNode *n){
    if(!n) return NULL;
    TNode * gpNode = grand_parent(n);
    if(!gpNode) return NULL;
    if(n->parent == gpNode->left)
        return gpNode->right;
    else
        return gpNode->left;
}

TNode* sibling(const TNode* n){
    if(!n || !n->parent) return NULL;
    if(n->parent->left == n){
        return n->parent->right;
    }else{
        return n->parent->left;
    }
}

TNode* new_node(int value){
    TNode* node = (TNode*) malloc(sizeof(TNode));
    if(!node){
        perror("create node error.");
        return NULL;
    }

    node->value = value;
    node->color = RED;
    node->left = node->parent = node->right = NULL;

    return node;
}

RbTree* new_tree(){
    RbTree* tree = (RbTree*) malloc(sizeof(RbTree));
    tree->root = NULL;
    return tree;
}

/**
 *  左旋
 *         n1                                    n1
 *        /  \                                  /  \
 *       n2  n8                                n2  n8
 *      /      \                              /      \
 *     N3      n9  --> left_rotate(N3) -->   N5      n9
 *    /  \                                  /  \
 *   n4  N5                                N3  N7
 *       / \                              /  \
 *     N6  N7                            n4  N6
 */ 
void left_rotate(RbTree *tree, TNode* node){
    TNode * pNode = node->parent;
    TNode * rNode = node->right;

    if(!pNode){
        tree->root = rNode;
    } else {
        if(node == pNode->left)
            pNode->left = rNode;
        else
            pNode->right = rNode;
    }
    
    rNode->parent = pNode;

    node->right = rNode->left;
    if(rNode->left) rNode->left->parent = node;
    rNode->left = node;
    node->parent = rNode; 
}

/**
 *  右旋
 *         n1                                    n1
 *        /  \                                  /  \
 *       n2  n8                                n2  n8
 *      /      \                              /      \
 *     n3      N9  --> right_rotate(N9) -->  n3      N7
 *             / \                                  /  \
 *            N7 n5                                N6  N9
 *           /  \                                      / \
 *          N6  N4                                    N4 n5
 */ 
void right_rotate(RbTree *tree, TNode* node){
    TNode * pNode = node->parent;
    TNode * lNode = node->left;

    if(!pNode){
        tree->root = node;
    } else {
        if(node == pNode->right)
            pNode->right = lNode;
        else
            pNode->left = lNode;
    }

    lNode->parent = pNode;

    node->left = lNode->right;
    if(lNode->right) lNode->right->parent = node;
    lNode->right = node;
    node->parent = lNode;
}

void insert_adjust(RbTree *tree, TNode *node){
    // case 1 如果 node 的 parent 为空，则表示是根节点，直接改为黑节点（BLACK）即可；
    if(node->parent == NULL){
        node->color = BLACK;
    } else {
        if (node->parent->color == BLACK) {
            // case 2，节点的父节点是黑色节点，则不需要做任何移动，因为对任何规则都没有破坏
            // nothing need to do.
        } else 
        // case 3, 节点的父节点是红色节点，那么就违反了：连续两个红色节点的性质, 且必然有 grand parent
        if (node->parent->color == RED){
            TNode* gNode = grand_parent(node);
            TNode* uNode = uncle(node);
            
            /**
            *叔叔节点U都是红色节点，那么就：
            *   祖父节点变为红色。
            *   把父节点变为黑色。
            *   把叔叔节点变为黑色。
            */
            if(uNode && uNode->color == RED){
                gNode->color = RED;
                node->parent->color = BLACK;
                uNode->color = BLACK;

                // 由于 grand parent 变为了 RED，则有可能破坏了根节点为黑节点以及不能连续两个红节点的性质，
                // 所以需要对其重新进行调整，其他的已经满足了
                insert_adjust(tree, gNode);
            } else {
                // case 4，uncle 节点为 null 或者 叔叔节点是黑色的情况，这种情况下就需要通过旋转来保持规则
                // 4.1 新节点N是父节点P的右孩子，而P是其父节点的左孩子，那么左旋P。
                if(node == node->parent->right && node->parent == gNode->left){
                    left_rotate(tree, node->parent);
                    node = node->left;
                }else
                // 4.2 新结点N是父节点P的左孩子，而P是其父节点的右孩子，那么右旋P。
                if(node == node->parent->left && node->parent == gNode->right){
                    right_rotate(tree, node->parent); // 右旋和上面的左旋是反着的
                    node = node->right;
                }
                
                // case 5，经过了上面的旋转之后，就会只会出现以下两种情况中的一种，即新加入的节点和其父节点都各自为
                // 其父节点的左节点或者右节点，此时再将其父节点改为黑色，grand parent 节点改为红色
                // 再经过一次旋转，就能满足红黑树的性质
                node->parent->color = BLACK;
                gNode->color = RED;
                // 5.1 同为右节点
                if(node == node->parent->right && node->parent == gNode->right){
                    left_rotate(tree, gNode);
                }else 
                // 5.2 同为左节点
                if(node == node->parent->left && node->parent == gNode->left){
                    right_rotate(tree, gNode);
                }
            }
            
        }
    }
}

int insert(RbTree *tree, int value){
    if(!tree) return 0;
    TNode *node = new_node(value);
    if(!tree->root){
        tree->root = node;
    } else {
        TNode *p, *t = tree->root;
        while(t){
            if(value == t->value) {
                printf("value[%d] exists.", value);
                return 0;
            }
            p = t;
            if(value > t->value){
                t = t->right;
            } else {
                t = t->left;
            }
        }

        if(value > p->value){
            p->right = node;
        }else{
            p->left = node;
        }
        
        node->parent = p;
    }

    // 调整树节点，使其保持红黑树的性质
    insert_adjust(tree, node);

    return 1;
}

typedef enum Order{
    pre,    // 先序
    post,   // 后序
    middle  // 中序
} Order;

// 递归中序遍历树
void visit_node(const TNode * n, Order order){
    if(!n) return;
    switch(order){
        case middle:
            printf("%d[%s],", n->value, n->color == RED? "RED" : "BLACK");
            visit_node(n->left, order);
            visit_node(n->right, order);
            break;
        case post:
            visit_node(n->left, order);
            visit_node(n->right, order);
            printf("%d[%s],", n->value, n->color == RED? "RED" : "BLACK");
            break;
        case pre:
            visit_node(n->left, order);
            printf("%d[%s],", n->value, n->color == RED? "RED" : "BLACK");
            visit_node(n->right, order);
            break;
    }
}

void visit_tree(const RbTree * tree, Order order){
    printf("tree:");
    if(!tree || !tree->root){
        printf("empty tree.\n");
        return;
    }

    visit_node(tree->root, order);
    printf("\n");
}

int main(int argc, char const *argv[]) {
    printf("hello world, red-black tree. \n");
    RbTree * tree = new_tree();

    insert(tree,1);
    insert(tree,2);
    insert(tree,3);
    insert(tree,4);
    insert(tree,5);
    insert(tree,12);
    insert(tree,11);
    visit_tree(tree, middle);
    return 0;
}
