
#ifndef __FLINT_UTF8_DICT_NODE_H
#define __FLINT_UTF8_DICT_NODE_H

#include "flint_dict_node.h"

class Utf8DictNode : public DictNode<Utf8DictNode> {
private:    
    uint32_t hash;
    char value[];
public:
    uint32_t getHashKey(void) const;
    int32_t compareKey(const char *key, uint16_t length) const;
    int32_t compareKey(DictNode<Utf8DictNode> *other) const;

    const char *getValue() const;
private:
    Utf8DictNode(void);
    Utf8DictNode(const char *txt, uint16_t length);
    Utf8DictNode(const Utf8DictNode &) = delete;
    void operator=(const Utf8DictNode &) = delete;

    friend class Flint;
};

#endif /* __FLINT_UTF8_DICT_NODE_H */
