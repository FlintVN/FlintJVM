
#ifndef __FLINT_LIST_H
#define __FLINT_LIST_H

#include <concepts>

class ListNode {
public:
    void *onwerList;
    ListNode *next;
    ListNode *prev;
protected:
    ListNode(void) : onwerList(NULL), next(NULL), prev(NULL) {

    };
private:
    ListNode(const ListNode &) = delete;
    void operator=(const ListNode &) = delete;
};

template <typename T>
requires std::derived_from<T, ListNode>
class FList {
private:
    ListNode *root;
public:
    FList(void) : root(NULL) {

    }

    void add(T *node) {
        node->onwerList = (void *)this;
        node->prev = NULL;
        node->next = root;
        if(root != NULL)
            root->prev = node;
        root = node;
    }

    void remove(T *node) {
        if(node->onwerList != (void *)this)
            return;
        ListNode *prev = node->prev;
        ListNode *next = node->next;
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

    void forEach(void (*func)(T *item)) {
        for(ListNode *node = root; node != NULL;) {
            ListNode *nextNode = node->next;
            func((T *)node);
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
