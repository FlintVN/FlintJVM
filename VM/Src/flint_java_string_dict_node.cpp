
#include "flint_common.h"
#include "flint_java_string_dict_node.h"

JStringDictNode::JStringDictNode(JString *str) : DictNode<JStringDictNode>(), str(str) {

}

uint32_t JStringDictNode::getHashKey(void) const {
    return str->getHashCode();
}

int32_t JStringDictNode::compareKey(const char *key, uint16_t length) const {
    return str->compareTo(key, length);
}

int32_t JStringDictNode::compareKey(DictNode<JStringDictNode> *other) const {
    return str->compareTo(((JStringDictNode *)other)->str);
}

JString *JStringDictNode::getString(void) const {
    return str;
}
