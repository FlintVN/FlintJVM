
#ifndef __FLINT_UTF8_DICT_NODE_H
#define __FLINT_UTF8_DICT_NODE_H

#include "flint_dictionary.h"

class Utf8DictNode : public DictNode {
private:
    uint32_t hash;
    char value[];
public:
    uint32_t getHashKey(void) const override;
    int32_t compareKey(const char *key, uint16_t length) const override;
    int32_t compareKey(DictNode *other) const override;

    const char *getValue() const;
private:
    Utf8DictNode(void);
    Utf8DictNode(const char *txt, uint16_t length);
    Utf8DictNode(const Utf8DictNode &) = delete;
    void operator=(const Utf8DictNode &) = delete;

    friend class Flint;
};

#endif /* __FLINT_UTF8_DICT_NODE_H */
