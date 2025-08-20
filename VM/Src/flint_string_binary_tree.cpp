
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_string_binary_tree.h"

FlintStringBinaryTree::FlintStringNode::FlintStringNode(FlintJavaString &value) : left(NULL_PTR), right(NULL_PTR), height(1), value(value) {

}

FlintStringBinaryTree::FlintStringBinaryTree(void) : root(NULL_PTR) {

}

uint32_t FlintStringBinaryTree::getHeight(FlintStringNode *node) {
    return node ? node->height : 0;
}

int32_t FlintStringBinaryTree::getBalance(FlintStringNode *node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void FlintStringBinaryTree::updateHeight(FlintStringNode *node) {
    if(node) {
        uint32_t leftHeight = getHeight(node->left);
        uint32_t rightHeight = getHeight(node->right);
        node->height = 1 + FLINT_MAX(leftHeight, rightHeight);
    }
}

FlintStringBinaryTree::FlintStringNode *FlintStringBinaryTree::rotateRight(FlintStringNode *y) {
    FlintStringNode *x = y->left;
    FlintStringNode *T = x->right;

    x->right = y;
    y->left = T;

    updateHeight(y);
    updateHeight(x);

    return x;
}

FlintStringBinaryTree::FlintStringNode *FlintStringBinaryTree::rotateLeft(FlintStringNode *x) {
    FlintStringNode *y = x->right;
    FlintStringNode *T = y->left;

    y->left = x;
    x->right = T;

    updateHeight(x);
    updateHeight(y);

    return y;
}

FlintStringBinaryTree::FlintStringNode *FlintStringBinaryTree::balance(FlintStringNode *node) {
    if(!node)
        return node;
    updateHeight(node);
    int balanceFactor = getBalance(node);
    if(balanceFactor > 1) {
        if(getBalance(node->left) >= 0)
            return rotateRight(node);
        else {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
    }
    else if(balanceFactor < -1) {
        if(getBalance(node->right) <= 0)
            return rotateLeft(node);
        else {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
    }
    return node;
}

FlintStringBinaryTree::FlintStringNode *FlintStringBinaryTree::insert(FlintStringNode *rootNode, FlintJavaString &value) {
    if(!rootNode) {
        FlintStringNode *stringNode = (FlintStringNode *)Flint::malloc(sizeof(FlintStringNode));
        new (stringNode)FlintStringNode(value);
        return stringNode;
    }
    int8_t compareResult = value.compareTo(rootNode->value);
    if(compareResult < 0)
        rootNode->left = insert(rootNode->left, value);
    else if(compareResult > 0)
        rootNode->right = insert(rootNode->right, value);
    else
        return rootNode;
    return balance(rootNode);
}

FlintJavaString &FlintStringBinaryTree::add(FlintJavaString &value) {
    root = insert(root, value);
    return value;
}

FlintJavaString *FlintStringBinaryTree::find(FlintJavaString &value) const {
    FlintStringNode *node = root;
    while(node) {
        int8_t compareResult = value.compareTo(node->value);
        if(compareResult == 0)
            return &node->value;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
}

FlintJavaString *FlintStringBinaryTree::find(const FlintConstUtf8 &utf8) const {
    FlintStringNode *node = root;
    while(node) {
        int8_t compareResult = node->value.compareTo(utf8);
        if(compareResult == 0)
            return &node->value;
        else if(compareResult < 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
}

void FlintStringBinaryTree::forEach(FlintStringNode *node, void (*func)(FlintJavaString &item)) {
    if(node) {
        forEach(node->left, func);
        forEach(node->right, func);
        func(node->value);
    }
}

void FlintStringBinaryTree::forEach(void (*func)(FlintJavaString &)) {
    forEach(root, func);
}

void FlintStringBinaryTree::freeNode(FlintStringNode *node) {
    if(node) {
        freeNode(node->left);
        freeNode(node->right);
        Flint::free(node);
    }
}

void FlintStringBinaryTree::clear(void) {
    freeNode(root);
    root = NULL_PTR;
}
