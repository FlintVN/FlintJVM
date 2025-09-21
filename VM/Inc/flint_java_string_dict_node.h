
#ifndef __FLINT_JAVA_STRING_DICT_NODE_H
#define __FLINT_JAVA_STRING_DICT_NODE_H

#include "flint_dictionary.h"
#include "flint_java_string.h"

class JStringDictNode : public DictNode {
private:
    JString *str;
public:
    uint32_t getHashKey(void) const override;
    int32_t compareKey(const char *key, uint16_t length) const override;
    int32_t compareKey(DictNode *other) const override;

    JString *getString(void) const;
private:
    JStringDictNode(JString *str);
    JStringDictNode(const JStringDictNode &) = delete;
    void operator=(const JStringDictNode &) = delete;

    friend class Flint;
};

#endif /* __FLINT_JAVA_STRING_DICT_NODE_H */
