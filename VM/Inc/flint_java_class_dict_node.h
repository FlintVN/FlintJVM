
#ifndef __FLINT_JAVA_CLASS_DICT_NODE_H
#define __FLINT_JAVA_CLASS_DICT_NODE_H

#include "flint_dict_node.h"
#include "flint_java_class.h"

class JClassDictNode : public DictNode<JClassDictNode> {
private:
    JClass *cls;
    uint32_t hash;
public:
    uint32_t getHashKey(void) const;
    int32_t compareKey(const char *key, uint16_t length) const;
    int32_t compareKey(DictNode<JClassDictNode> *other) const;

    JClass *getClass(void) const;
private:
    JClassDictNode(JClass *cls);
    JClassDictNode(const JClassDictNode &) = delete;
    void operator=(const JClassDictNode &) = delete;

    friend class Flint;
};

#endif /* __FLINT_JAVA_CLASS_DICT_NODE_H */
