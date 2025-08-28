
#include "flint_common.h"
#include "flint_java_class_dict_node.h"

JClassDictNode::JClassDictNode(JClass *cls) : DictNode<JClassDictNode>(), cls(cls) {
    hash = Hash(cls->getTypeName());
}

uint32_t JClassDictNode::getHashKey(void) const {
    return hash;
}

int32_t JClassDictNode::compareKey(const char *key, uint16_t length) const {
    return (length > 0) ? strncmp(cls->getTypeName(), key, length) : strcmp(cls->getTypeName(), key);
}

int32_t JClassDictNode::compareKey(DictNode<JClassDictNode> *other) const {
    return strcmp(cls->getTypeName(), ((JClassDictNode *)other)->cls->getTypeName());
}

JClass *JClassDictNode::getClass(void) const {
    return cls;
}
