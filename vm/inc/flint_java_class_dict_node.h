
#ifndef __FLINT_JAVA_CLASS_DICT_NODE_H
#define __FLINT_JAVA_CLASS_DICT_NODE_H

#include "flint_dictionary.h"
#include "flint_java_class.h"

class JClassDictNode : public DictNode {
private:
    JClass *cls;
    uint32_t hash;
public:
    uint32_t getHashKey(void) const override;
    int32_t compareKey(const char *key, uint16_t length) const override;
    int32_t compareKey(DictNode *other) const override;

    JClass *getClass(void) const;
private:
    JClassDictNode(JClass *cls);
    JClassDictNode(const JClassDictNode &) = delete;
    void operator=(const JClassDictNode &) = delete;

    friend class Flint;
};

#endif /* __FLINT_JAVA_CLASS_DICT_NODE_H */
