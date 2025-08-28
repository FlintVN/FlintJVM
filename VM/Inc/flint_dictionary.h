

#ifndef __FLINT_DICTIONARY_H
#define __FLINT_DICTIONARY_H

#include "flint.h"
#include "flint_dict_node.h"
#include "flint_common.h"

template <class T>
class FDict {
private:
    DictNode<T> *root;

    static uint32_t getHeight(DictNode<T> *node) {
        return node ? node->height : 0;
    }

    static int32_t getBalance(DictNode<T> *node) {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    static void updateHeight(DictNode<T> *node) {
        if(node) {
            uint32_t leftHeight = getHeight(node->left);
            uint32_t rightHeight = getHeight(node->right);
            node->height = 1 + ((leftHeight > rightHeight) ? leftHeight : rightHeight);
        }
    }

    static DictNode<T> *rotateRight(DictNode<T> *y) {
        DictNode<T> *x = y->left;
        DictNode<T> *t = x->right;

        x->right = y;
        y->left = t;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    static DictNode<T> *rotateLeft(DictNode<T> *x) {
        DictNode<T> *y = x->right;
        DictNode<T> *t = y->left;

        y->left = x;
        x->right = t;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    static DictNode<T> *balance(DictNode<T> *node) {
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

    static DictNode<T> *insert(DictNode<T> *root, DictNode<T> *node) {
        if(!root) return node;
        int32_t cmp = node->getHashKey() - root->getHashKey();
        if(cmp == 0) cmp = node->compareKey(root);
        if(cmp < 0) root->left = insert(root->left, node);
        else if(cmp > 0) root->right = insert(root->right, node);
        else return root;
        return balance(root);
    }

    static void forEach(DictNode<T> *node, void (*func)(DictNode<T> *item)) {
        if(node) {
            forEach(node->left, func);
            forEach(node->right, func);
            func(node);
        }
    }

    FDict(void) : root(NULL) {

    }

    T *find(const char *key, uint16_t length = 0xFFFF) {
        if(root == NULL) return NULL;
        uint32_t hash = Hash(key, length);
        DictNode<T> *node = root;
        while(node) {
            int32_t cmp = node->getHashKey() - hash;
            if(cmp == 0) cmp = node->compareKey(key, length);
            if(cmp == 0) return (T *)node;
            else if(cmp < 0) node = node->right;
            else node = node->left;
        }
        return NULL;
    }

    T *find(DictNode<T> *value) {
        if(root == NULL) return NULL;
        uint32_t hash = value->getHashKey();
        DictNode<T> *node = root;
        while(node) {
            int32_t cmp = node->getHashKey() - hash;
            if(cmp == 0) cmp = node->compareKey(value);
            if(cmp == 0) return (T *)node;
            else if(cmp < 0) node = node->right;
            else node = node->left;
        }
        return NULL;
    }

    void add(DictNode<T> *node) {
        root = insert(root, node);
    }

    void forEach(void (*func)(DictNode<T> *item)) {
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
