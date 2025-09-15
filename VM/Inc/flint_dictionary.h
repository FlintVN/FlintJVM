

#ifndef __FLINT_DICTIONARY_H
#define __FLINT_DICTIONARY_H

#include <concepts>
#include "flint_common.h"

class DictNode {
public:
    DictNode *left;
    DictNode *right;
    uint32_t height;
protected:
    DictNode(void) : left(NULL), right(NULL), height(0) {

    }
public:
    virtual uint32_t getHashKey(void) const = 0;
    virtual int32_t compareKey(const char *key, uint16_t length = 0xFFFF) const = 0;
    virtual int32_t compareKey(DictNode *other) const = 0;
private:
    DictNode(const DictNode &) = delete;
    void operator=(const DictNode &) = delete;
};

template <class T>
requires std::derived_from<T, DictNode>
class FDict {
private:
    DictNode *root;

    static uint32_t getHeight(DictNode *node) {
        return node ? node->height : 0;
    }

    static int32_t getBalance(DictNode *node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    static void updateHeight(DictNode *node) {
        if(node) {
            uint32_t leftHeight = getHeight(node->left);
            uint32_t rightHeight = getHeight(node->right);
            node->height = 1 + ((leftHeight > rightHeight) ? leftHeight : rightHeight);
        }
    }

    static DictNode *rotateRight(DictNode *y) {
        DictNode *x = y->left;
        DictNode *t = x->right;

        x->right = y;
        y->left = t;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    static DictNode *rotateLeft(DictNode *x) {
        DictNode *y = x->right;
        DictNode *t = y->left;

        y->left = x;
        x->right = t;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    static DictNode *balance(DictNode *node) {
        if(!node)
            return node;
        updateHeight(node);
        int balanceFactor = getBalance(node);
        if(balanceFactor > 1) {
            if(getBalance(node->left) >= 0) return rotateRight(node);
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        else if(balanceFactor < -1) {
            if(getBalance(node->right) <= 0) return rotateLeft(node);
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    static DictNode *insert(DictNode *root, DictNode *node) {
        if(!root) return node;
        int32_t cmp = node->getHashKey() - root->getHashKey();
        if(cmp == 0) cmp = node->compareKey(root);
        if(cmp < 0) root->left = insert(root->left, node);
        else if(cmp > 0) root->right = insert(root->right, node);
        else return root;
        return balance(root);
    }

    static void forEach(DictNode *node, void (*func)(T *item)) {
        if(node) {
            forEach(node->left, func);
            forEach(node->right, func);
            func((T *)node);
        }
    }

    FDict(void) : root(NULL) {

    }

    T *find(const char *key, uint16_t length = 0xFFFF) {
        if(root == NULL) return NULL;
        uint32_t hash = Hash(key, length);
        DictNode *node = root;
        while(node) {
            int32_t cmp = node->getHashKey() - hash;
            if(cmp == 0) cmp = node->compareKey(key, length);
            if(cmp == 0) return (T *)node;
            else if(cmp < 0) node = node->right;
            else node = node->left;
        }
        return NULL;
    }

    T *find(T *value) {
        if(root == NULL) return NULL;
        uint32_t hash = value->getHashKey();
        DictNode *node = root;
        while(node) {
            int32_t cmp = node->getHashKey() - hash;
            if(cmp == 0) cmp = node->compareKey(value);
            if(cmp == 0) return (T *)node;
            else if(cmp < 0) node = node->right;
            else node = node->left;
        }
        return NULL;
    }

    void add(DictNode *node) {
        root = insert(root, node);
    }

    void forEach(void (*func)(T *item)) {
        forEach(root, func);
    }

    void clear(void) {
        root = NULL;
    }
private:
    FDict(const FDict<T> &) = delete;
    void operator=(const FDict<T> &) = delete;

    friend class Flint;
};

#endif /* __FLINT_DICTIONARY_H */
