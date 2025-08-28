
#ifndef __FLINT_LIST_H
#define __FLINT_LIST_H

#include "flint_list_node.h"

template <class T>
class FList {
private:
    ListNode<T> *root;
public:
    FList(void) : root(NULL) {

    }

    void add(ListNode<T> *node) {
        node->onwerList = (void *)this;
        node->prev = NULL;
        node->next = root;
        if(root != NULL)
            root->prev = node;
        root = node;
    }

    void remove(ListNode<T> *node) {
        if(node->onwerList != (void *)this)
            return;
        ListNode<T> *prev = node->prev;
        ListNode<T> *next = node->next;
        if(prev != NULL)
            prev->next = next;
        else
            root = next;
        if(next)
            next->prev = prev;
        node->onwerList = NULL;
        node->prev = NULL;
        node->next = NULL;
    }

    void forEach(void (*func)(ListNode<T> *item)) {
        for(ListNode<T> *node = root; node != NULL;) {
            ListNode<T> *nextNode = node->next;
            func(node);
            node = nextNode;
        }
    }

    void clear(void) {
        root = NULL;
    }
private:
    FList(const FList<T> &) = delete;
    void operator=(const FList<T> &) = delete;

    friend class Flint;
};

#endif /* __FLINT_LIST_H */
