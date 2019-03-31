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
        tree->root = lNode;
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
void visit_node(const TNode * n, Order order, int depth){
    int i = 0;
    if(!n) return;
    switch(order){
        case middle:
            for(i = 0;i < depth; ++i) {
                if(i == depth - 1)
                    printf("|----");
                else
                    printf("\t");
            }

            printf("-> %d[%s,%s] \n", n->value, 
                n->parent?(n == n->parent->left?"l":"r"):"_r",
                n->color == RED? "RED" : "BLACK");
            visit_node(n->left, order, depth+1);
            visit_node(n->right, order, depth+1);
            break;
        case post:
            visit_node(n->left, order, depth+1);
            visit_node(n->right, order, depth+1);
            printf("%d[%s,%s] \n", n->value, 
                n->parent?(n == n->parent->left?"l":"r"):"_r",
                n->color == RED? "RED" : "BLACK");
            break;
        case pre:
            visit_node(n->left, order, depth+1);
            printf("%d[%s,%s] \n", n->value, 
                n->parent?(n == n->parent->left?"l":"r"):"_r",
                n->color == RED? "RED" : "BLACK");
            visit_node(n->right, order, depth+1);
            break;
    }
}

void visit_tree(const RbTree * tree, Order order){
    printf("tree: \n");
    if(!tree || !tree->root){
        printf("empty tree.\n");
        return;
    }

    visit_node(tree->root, order, 0);
    printf("\n");
}

/**
 * 来自 Linux kernel 2.6.0 rbtree.c
 * 进行删除时的颜色调整
 * @parentNode 替换“被删除元素”的元素的初始父节点
 * @childNode  替换“被删除元素”的元素的初始子节点
 */ 
void delete_adjust(RbTree * tree, TNode* parentNode, TNode* childNode){
    // case 1 如果 childNode 已经是根节点，则只需要将其颜色改为黑色即可，这直接会执行 while 循环后面的 if 语句，将颜色改为 BLACK
    TNode * brother;

    // 如果子节点是空或者子节点是黑色 且 子节点不是根节点(只要子节点不是根节点，则父节点必然存在)
    while((!childNode || childNode->color == BLACK) && childNode != tree->root){
        // 如果子节点是左孩子
        if(childNode == parentNode->left) {
            brother = parentNode->right;
            /**
             * case 1
             *  x是”黑+黑”节点，x的兄弟节点是红色。(此时x的父节点和x的兄弟节点的子节点都是黑节点)。
             *  (01) 将x的兄弟节点设为“黑色”。
             *  (02) 将x的父节点设为“红色”。
             *  (03) 对x的父节点进行左旋。
             *  (04) 左旋后，重新设置x的兄弟节点。
            */
            if(brother->color == RED){
                brother->color = BLACK;
                parentNode->color = RED;
                left_rotate(tree, parentNode);
                // parentNode->right 会指向 brother 的左节点
                brother = parentNode->right;
            }

            /**
             * case 2 
             *   x的兄弟节点是黑色，x的兄弟节点的两个孩子都是黑色。
             *   (01) 将x的兄弟节点设为“红色”。
             *   (02) 设置“x的父节点”为“新的x节点”。
             */
            if((!brother->left || brother->left->color == BLACK) && (!brother->right || brother->right->color == BLACK)){
                brother->color = RED;
                childNode = parentNode;
                parentNode = parentNode->parent;
            }else{
                /**
                 * case 3
                 * x的兄弟节点是黑色；x的兄弟节点的左孩子是红色，右孩子是黑色的。
                 *   (01) 将x兄弟节点的左孩子设为“黑色”。
                 *   (02) 将x兄弟节点设为“红色”。
                 *   (03) 对x的兄弟节点进行右旋。
                 *   (04) 右旋后，重新设置x的兄弟节点。
                 */
                if(!brother->right || brother->right->color == BLACK){
                    register TNode* oLeft;
                    if((oLeft = brother->left))
                        oLeft->color = BLACK;
                    brother->color = RED;
                    right_rotate(tree, brother);
                    brother = parentNode->right;
                }

                /**
                 * case 4
                 * x的兄弟节点是黑色；x的兄弟节点的右孩子是红色的，x的兄弟节点的左孩子任意颜色。
                 *   (01) 将x父节点颜色赋值给x的兄弟节点。
                 *   (02) 将x父节点设为“黑色”。
                 *   (03) 将x兄弟节点的右子节设为“黑色”。
                 *   (04) 对x的父节点进行左旋。
                 *   (05) 设置“x”为“根节点”。 
                 */
                brother->color = parentNode->color;
                parentNode->color = BLACK;
                if(brother->right)
                    brother->right->color = BLACK;
                left_rotate(tree, parentNode);
                childNode = tree->root;
                break;
            }
        }else{
            brother = parentNode->left;
            // case 2，如果兄弟节点为红色，
            if(brother->color == RED){
                brother->color = BLACK;
                parentNode->color = RED;
                right_rotate(tree, parentNode);
                // parentNode->right 会指向 brother 的左节点
                brother = parentNode->left;
            }
            if((!brother->right || brother->right->color == BLACK) && (!brother->left || brother->left->color == BLACK)){
                brother->color = RED;
                childNode = parentNode;
                parentNode = parentNode->parent;
            }else{
                if(!brother->left || brother->left->color == BLACK){
                    register TNode* oRight;
                    if((oRight = brother->right))
                        oRight->color = BLACK;
                    brother->color = RED;
                    left_rotate(tree, brother);
                    brother = parentNode->left;
                }

                brother->color = parentNode->color;
                parentNode->color = BLACK;
                if(brother->left)
                    brother->left->color = BLACK;
                right_rotate(tree, parentNode);
                childNode = tree->root;
                break;
            }
        }
    }

    // 子节点的颜色调成黑色，让当前分支的黑色个数保持不变（因为删除的是黑色节点）
    if(childNode) childNode->color = BLACK;
}

TNode * search_node(RbTree * tree, int value){
    if(!tree) return NULL;
    TNode* target = tree->root;
    while(target && target->value != value){
        if(value < target->value){
            target = target->left;
        }else{
            target = target->right;
        }
    }

    return target;
}

/**
 * 删除节点
 * 核心思路：（https://zh.wikipedia.org/wiki/%E7%BA%A2%E9%BB%91%E6%A0%91）
 * 如果需要删除的节点有两个儿子，那么问题可以被转化成删除另一个只有一个儿子的节点的问题（为了表述方便，这里所指的儿子，为非叶子节点的儿子）。
 * 对于二叉查找树，在删除带有两个非叶子儿子的节点的时候，我们要么找到它左子树中的最大元素、要么找到它右子树中的最小元素，并把它的值转移到要
 * 删除的节点中（如在这里所展示的那样）。我们接着删除我们从中复制出值的那个节点，它必定有少于两个非叶子的儿子。因为只是复制了一个值（没有复
 * 制颜色），不违反任何性质，这就把问题简化为如何删除最多有一个儿子的节点的问题。它不关心这个节点是最初要删除的节点还是我们从中复制出值的那个节点。
 * 
 * 在本文余下的部分中，我们只需要讨论删除只有一个儿子的节点（如果它两个儿子都为空，即均为叶子，我们任意将其中一个看作它的儿子）。
 * 如果我们删除一个红色节点（此时该节点的儿子将都为叶子节点），它的父亲和儿子一定是黑色的。所以我们可以简单的用它的黑色儿子替换它，并不会破坏性质3和性质4。
 * 通过被删除节点的所有路径只是少了一个红色节点，这样可以继续保证性质5。另一种简单情况是在被删除节点是黑色而它的儿子是红色的时候。如果只是去除这个黑色节点，
 * 用它的红色儿子顶替上来的话，会破坏性质5，但是如果我们重绘它的儿子为黑色，则曾经通过它的所有路径将通过它的黑色儿子，这样可以继续保持性质5。
 * 
 * 需要进一步讨论的是在要删除的节点和它的儿子二者都是黑色的时候，这是一种复杂的情况（这种情况下该结点的两个儿子都是叶子结点，否则若其中一个儿子是黑色非叶子
 * 结点，另一个儿子是叶子结点，那么从该结点通过非叶子结点儿子的路径上的黑色结点数最小为2，而从该结点到另一个叶子结点儿子的路径上的黑色结点数为1，违反了性质5）。
 * 我们首先把要删除的节点替换为它的儿子。出于方便，称呼这个儿子为N（在新的位置上），称呼它的兄弟（它父亲的另一个儿子）为S。在下面的示意图中，我们还是使用P称呼
 * N的父亲，SL称呼S的左儿子，SR称呼S的右儿子。
 * 
 * @return 返回删除的节点数量
 */
int delete_node(RbTree * tree, int value){
    // 1. 找到要删除的节点
    TNode * delNode = search_node(tree, value);
    if(!delNode){ // not found, nothing need to do.
        return 0;
    }

    // 2. 如果左右子树都不为空的情况下，我们需要找到替换的节点，进而将包含两个非空子节点的问题，转换成最多包含一个非空节点的问题，
    // 这里有两种方案，一种是找左子树的最大值，另一种是找右子树的最小值，在这里，我们查找右子树的最小值
    TNode * child = NULL,* parent = NULL;
    Color delColor;
    if(delNode->right && delNode->left){
        TNode* minRightNode = delNode->right;
        // 找右子树最小节点
        while(minRightNode->left) minRightNode = minRightNode->left;

        delColor = minRightNode->color;
        parent = minRightNode->parent;

        child = minRightNode->right; // minRightNode->left must be NULL
        if(child) { // 如果子节点不为空，移除 minRightNode 所在的位置
            // 则将子节点的父节点指向 minRightNode 的父节点
            child->parent = minRightNode->parent;
        }

        // 再将父节点的父节点指向子节点
        if(minRightNode->parent->left == minRightNode){
            minRightNode->parent->left = child;
        }else{
            minRightNode->parent->right = child;
        }

        // 将要被删除的节点和右子树最小节点进行位置替换，除了值，即让右子树最小节点到达被删除节点的位置
        minRightNode->parent = delNode->parent;
        minRightNode->left = delNode->left;
        minRightNode->right = delNode->right;
        minRightNode->color = delNode->color;

        if(delNode->parent){
            if(delNode->parent->left == delNode){
                delNode->parent->left = minRightNode;
            }else{
                delNode->parent->right = minRightNode;
            }
        }else{
            tree->root = minRightNode;
        }

        delNode->left->parent = minRightNode;
        if(delNode->right)
            delNode->right->parent = minRightNode;
    }else{
        if(!delNode->left) child = delNode->right;
        else if(!delNode->right) child = delNode->left;

        parent = delNode->parent;
        delColor = delNode->color;

        // child 节点替换 delNode 的位置
        if(child){
            child->parent = parent;
        }
        // delNode 为跟节点时，parent 可能为空
        if(parent){
            if(delNode == parent->left) 
                parent->left = child;
            else 
                parent->right = child;
        }else{
            tree->root = child;
        }
    }

    // 若删除的节点的颜色是红色，则不需要关注，只有为黑色的节点，才需要进行颜色的调整
    if(delColor == BLACK)
        delete_adjust(tree, parent, child);

    free(delNode);
    delNode = NULL;
    return 1;
}

int main(int argc, char const *argv[]) {
    printf("hello world, red-black tree. \n");
    RbTree * tree = new_tree();
    
    while(1){
        printf("input insert number...\n");
        int nbr = 0;
        scanf("%d", &nbr);
        insert(tree,nbr);
        visit_tree(tree, middle);

        if(nbr == -1) break;
    }

    printf("begin to delete... \n");
    while(1){
        printf("input deleted number...\n");
        int nbr = 0;
        scanf("%d", &nbr);
        int r = delete_node(tree, nbr);
        if(r == 0){
            printf("not found %d\n", nbr);
        }
        visit_tree(tree, middle);
    }
    /*delete_node(tree, 11);
    visit_tree(tree, middle);
    delete_node(tree, 12);
    visit_tree(tree, middle);
    delete_node(tree, 5);
    visit_tree(tree, middle);
    delete_node(tree, 4);
    visit_tree(tree, middle);
    delete_node(tree, 3);
    visit_tree(tree, middle);
    delete_node(tree, 2);
    visit_tree(tree, middle);
    delete_node(tree, 1);
    visit_tree(tree, middle);*/
    return 0;
}
