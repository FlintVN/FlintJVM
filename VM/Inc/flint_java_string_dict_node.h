
#ifndef __FLINT_JAVA_STRING_DICT_NODE_H
#define __FLINT_JAVA_STRING_DICT_NODE_H

#include "flint_dict_node.h"
#include "flint_java_string.h"

class JStringDictNode : public DictNode<JStringDictNode> {
private:
    JString *str;
public:
    uint32_t getHashKey(void) const;
    int32_t compareKey(const char *key, uint16_t length) const;
    int32_t compareKey(DictNode<JStringDictNode> *other) const;

    JString *getString(void) const;
private:
    JStringDictNode(JString *str);
    JStringDictNode(const JStringDictNode &) = delete;
    void operator=(const JStringDictNode &) = delete;

    friend class Flint;
};

#endif /* __FLINT_JAVA_STRING_DICT_NODE_H */
