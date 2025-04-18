#ifndef __FLINT_STRING_BINARY_TREE_H
#define __FLINT_STRING_BINARY_TREE_H

#include "flint_const_pool.h"
#include "flint_java_string.h"

class FlintStringBinaryTree {
private:
    struct FlintStringNode {
        FlintStringNode *left;
        FlintStringNode *right;
        uint32_t height;
        FlintJavaString &value;

        FlintStringNode(FlintJavaString &value);
    };

    FlintStringNode *root;

    FlintStringBinaryTree(const FlintStringBinaryTree &) = delete;
    void operator=(const FlintStringBinaryTree &) = delete;

    static uint32_t getHeight(FlintStringNode *node);
    static int32_t getBalance(FlintStringNode *node);
    static void updateHeight(FlintStringNode *node);
    static FlintStringNode *rotateRight(FlintStringNode *y);
    static FlintStringNode *rotateLeft(FlintStringNode *x);
    static FlintStringNode *balance(FlintStringNode *node);
    static FlintStringNode *createFlintConstUtf8Node(const char *text, uint32_t hash);
    static FlintStringNode *insert(FlintStringNode *rootNode, FlintJavaString &value);

    void forEach(FlintStringNode *node, void (*func)(FlintJavaString &item));
    void freeNode(FlintStringNode *node);
public:
    FlintStringBinaryTree(void);

    FlintJavaString &add(FlintJavaString &value);

    FlintJavaString *find(FlintJavaString &value) const;
    FlintJavaString *find(const FlintConstUtf8 &utf8) const;

    void forEach(void (*func)(FlintJavaString &));
    void clear(void);
};

#endif /* __FLINT_STRING_BINARY_TREE_H */
