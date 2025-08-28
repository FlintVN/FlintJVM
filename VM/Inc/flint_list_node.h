
#ifndef __FLINT_LIST_NODE_H
#define __FLINT_LIST_NODE_H

#include "flint_std.h"

template <class T>
class ListNode {
public:
    void *onwerList;
    ListNode<T> *next;
    ListNode<T> *prev;
protected:
    ListNode(void) : onwerList(NULL), next(NULL), prev(NULL) {

    };
private:
    ListNode(const ListNode<T> &) = delete;
    void operator=(const ListNode<T> &) = delete;
};

#endif /* __FLINT_LIST_NODE_H */
