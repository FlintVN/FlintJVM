
#ifndef __FLINT_DICT_NODE_H
#define __FLINT_DICT_NODE_H

#include <string.h>
#include "flint_std.h"

template <class T>
class DictNode {
public:
    DictNode<T> *left;
    DictNode<T> *right;
    uint32_t height;
protected:
    DictNode(void) : left(NULL), right(NULL), height(0) {

    }
public:
    virtual uint32_t getHashKey(void) const = 0;
    virtual int32_t compareKey(const char *key, uint16_t length = 0xFFFF) const = 0;
    virtual int32_t compareKey(DictNode<T> *other) const = 0;
private:
    DictNode(const DictNode<T> &) = delete;
    void operator=(const DictNode<T> &) = delete;
};

#endif /* __FLINT_DICT_NODE_H */
