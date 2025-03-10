#ifndef __FLINT_CONST_UTF8_BINARY_TREE_H
#define __FLINT_CONST_UTF8_BINARY_TREE_H

#include "flint_const_pool.h"

class FlintConstUtf8BinaryTree {
private:
    struct FlintConstUtf8Node {
        FlintConstUtf8Node *left;
        FlintConstUtf8Node *right;
        uint32_t height;
        FlintConstUtf8 value;
    };

    FlintConstUtf8Node *root;

    FlintConstUtf8BinaryTree(const FlintConstUtf8BinaryTree &) = delete;
    void operator=(const FlintConstUtf8BinaryTree &) = delete;

    static uint32_t getHeight(FlintConstUtf8Node *node);
    static int32_t getBalance(FlintConstUtf8Node *node);
    static void updateHeight(FlintConstUtf8Node *node);
    static FlintConstUtf8Node *rotateRight(FlintConstUtf8Node *y);
    static FlintConstUtf8Node *rotateLeft(FlintConstUtf8Node *x);
    static FlintConstUtf8Node *balance(FlintConstUtf8Node *node);
    static FlintConstUtf8Node *createFlintConstUtf8Node(const char *text, uint32_t hash, bool isTypeName);
    static FlintConstUtf8Node *insert(FlintConstUtf8Node *rootNode, const char *text, uint32_t hash, bool isTypeName, FlintConstUtf8Node **node);

    void freeNode(FlintConstUtf8Node *node);
public:
    FlintConstUtf8BinaryTree(void);

    FlintConstUtf8 &add(const char *text, uint16_t length, bool isTupeName);

    FlintConstUtf8 *find(const char *text, uint16_t length, bool isTupeName) const;

    void clear(void);
};

#endif /* __FLINT_CONST_UTF8_BINARY_TREE_H */
