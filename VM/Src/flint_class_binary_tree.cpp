
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_class_binary_tree.h"

FlintClassBinaryTree::FlintClassNode::FlintClassNode(FlintJavaClass &value) : left(NULL_PTR), right(NULL_PTR), height(0), value(value) {

}

FlintClassBinaryTree::FlintClassBinaryTree(void) : root(NULL_PTR) {

}

uint32_t FlintClassBinaryTree::getHeight(FlintClassNode *node) {
    return node ? node->height : 0;
}

int32_t FlintClassBinaryTree::getBalance(FlintClassNode *node) {
    return node ? getHeight(node->left) - getHeight(node->right) : 0;
}

void FlintClassBinaryTree::updateHeight(FlintClassNode *node) {
    if(node) {
        uint32_t leftHeight = getHeight(node->left);
        uint32_t rightHeight = getHeight(node->right);
        node->height = 1 + FLINT_MAX(leftHeight, rightHeight);
    }
}

FlintClassBinaryTree::FlintClassNode *FlintClassBinaryTree::rotateRight(FlintClassNode *y) {
    FlintClassNode *x = y->left;
    FlintClassNode *T = x->right;

    x->right = y;
    y->left = T;

    updateHeight(y);
    updateHeight(x);

    return x;
}

FlintClassBinaryTree::FlintClassNode *FlintClassBinaryTree::rotateLeft(FlintClassNode *x) {
    FlintClassNode *y = x->right;
    FlintClassNode *T = y->left;

    y->left = x;
    x->right = T;

    updateHeight(x);
    updateHeight(y);

    return y;
}

FlintClassBinaryTree::FlintClassNode *FlintClassBinaryTree::balance(FlintClassNode *node) {
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

FlintClassBinaryTree::FlintClassNode *FlintClassBinaryTree::insert(FlintClassNode *rootNode, FlintJavaClass &value) {
    if(!rootNode) {
        FlintClassNode *classNode = (FlintClassNode *)Flint::malloc(sizeof(FlintClassNode));
        if(classNode)
            new (classNode)FlintClassNode(value);
        return classNode;
    }
    int8_t compareResult = value.getName().compareTo(rootNode->value.getName());
    if(compareResult < 0) {
        FlintClassNode *node = insert(rootNode->left, value);
        if(node)
            return NULL_PTR;
        rootNode->left = node;
    }
    else if(compareResult > 0) {
        FlintClassNode *node = insert(rootNode->right, value);
        if(node == NULL_PTR)
            return NULL_PTR;
        rootNode->right = node;
    }
    else
        return rootNode;
    return balance(rootNode);
}

FlintResult<void> FlintClassBinaryTree::add(FlintJavaClass &value) {
    FlintClassNode *node = insert(root, value);
    if(node == NULL_PTR)
        return ERR_OUT_OF_MEMORY;
    root = node;
    return ERR_OK;
}

static int32_t compareClassName(const char *typeName1, uint16_t length, FlintJavaString &typeName2) {
    if(length != typeName2.getLength())
        return length - typeName2.getLength();
    if(typeName2.getCoder() != 0)
        return -1;
    const char *value = typeName2.getText();
    for(uint32_t i = 0; i < length; i++) {
        char c = (typeName1[i] == '/') ? '.' : typeName1[i];
        if(c != value[i])
            return c - value[i];
    }
    return 0;
}

FlintJavaClass *FlintClassBinaryTree::find(const char *text, uint16_t length) const {
    FlintClassNode *node = root;
    while(node) {
        int8_t compareResult = compareClassName(text, length, node->value.getName());
        if(compareResult == 0)
            return &node->value;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
}

FlintJavaClass *FlintClassBinaryTree::find(FlintJavaString &str) const {
    FlintClassNode *node = root;
    while(node) {
        int8_t compareResult = str.compareTo(node->value.getName());
        if(compareResult == 0)
            return &node->value;
        else if(compareResult > 0)
            node = node->right;
        else
            node = node->left;
    }
    return NULL_PTR;
}

void FlintClassBinaryTree::forEach(FlintClassNode *node, void (*func)(FlintJavaClass &item)) {
    if(node) {
        forEach(node->left, func);
        forEach(node->right, func);
        func(node->value);
    }
}

void FlintClassBinaryTree::forEach(void (*func)(FlintJavaClass &)) {
    forEach(root, func);
}

void FlintClassBinaryTree::freeNode(FlintClassNode *node) {
    if(node) {
        freeNode(node->left);
        freeNode(node->right);
        Flint::free(node);
    }
}

void FlintClassBinaryTree::clear(void) {
    freeNode(root);
    root = NULL_PTR;
}
