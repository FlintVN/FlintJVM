#ifndef __FLINT_CLASS_BINARY_TREE_H
#define __FLINT_CLASS_BINARY_TREE_H

#include "flint_const_pool.h"
#include "flint_java_class.h"

class FlintClassBinaryTree {
private:
    struct FlintClassNode {
        FlintClassNode *left;
        FlintClassNode *right;
        uint32_t height;
        JClass *value;

        FlintClassNode(JClass *value);
    };

    FlintClassNode *root;

    FlintClassBinaryTree(const FlintClassBinaryTree &) = delete;
    void operator=(const FlintClassBinaryTree &) = delete;

    static uint32_t getHeight(FlintClassNode *node);
    static int32_t getBalance(FlintClassNode *node);
    static void updateHeight(FlintClassNode *node);
    static FlintClassNode *rotateRight(FlintClassNode *y);
    static FlintClassNode *rotateLeft(FlintClassNode *x);
    static FlintClassNode *balance(FlintClassNode *node);
    static FlintClassNode *createFlintConstUtf8Node(const char *text, uint32_t hash);
    static FlintClassNode *insert(FlintClassNode *rootNode, JClass *value);

    void forEach(FlintClassNode *node, void (*func)(JClass *item));
    void freeNode(FlintClassNode *node);
public:
    FlintClassBinaryTree(void);

    FlintResult<void> add(JClass *value);

    JClass *find(const char *text, uint16_t length) const;
    JClass *find(JString *str) const;

    void forEach(void (*func)(JClass *));
    void clear(void);
};

#endif /* __FLINT_CLASS_BINARY_TREE_H */
