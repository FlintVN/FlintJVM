
#ifndef __FLINT_LIST_H
#define __FLINT_LIST_H

#include <concepts>

class ListNode {
public:
    void *ownerList;
    ListNode *next;
    ListNode *prev;
protected:
    ListNode(void) : ownerList(NULL), next(NULL), prev(NULL) {

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
        if(node->ownerList != NULL)
            ((FList<T> *)node->ownerList)->remove(node);
        node->ownerList = (void *)this;
        node->prev = NULL;
        node->next = root;
        if(root != NULL)
            root->prev = node;
        root = node;
    }

    void remove(T *node) {
        if(node->ownerList != (void *)this)
            return;
        ListNode *prev = node->prev;
        ListNode *next = node->next;
        if(prev != NULL)
            prev->next = next;
        else
            root = next;
        if(next)
            next->prev = prev;
        node->ownerList = NULL;
        node->prev = NULL;
        node->next = NULL;
    }

    bool isContain(T *node) {
        return (node->ownerList == (void *)this) && (root != NULL);
    }

    template<typename Func>
    requires std::invocable<Func, T *>
    void forEach(Func func) {
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
