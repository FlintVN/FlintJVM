
#include <string.h>
#include "flint_common.h"
#include "flint_utf8_dict_node.h"

Utf8DictNode::Utf8DictNode(void) : hash(0) {

}

Utf8DictNode::Utf8DictNode(const char *txt, uint16_t length) {
    hash = Hash(txt);
    char *val = value;
    while(*txt) *val++ = *txt++;
    *val = 0;
}

uint32_t Utf8DictNode::getHashKey(void) const {
    return hash;
}

int32_t Utf8DictNode::compareKey(const char *key, uint16_t length) const {
    return strncmp(value, key, length);
}

int32_t Utf8DictNode::compareKey(DictNode *other) const {
    return strcmp(value, ((Utf8DictNode *)other)->value);
}

const char *Utf8DictNode::getValue() const {
    return value;
}
