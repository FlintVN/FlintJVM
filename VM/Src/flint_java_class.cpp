
#include <string.h>
#include "flint_common.h"
#include "flint_java_class.h"

JClass::JClass(const char *typeName, ClassLoader *loader) : JObject(sizeof(FieldsData), NULL) {
    void **internalData = (void **)&getFields()[1];
    internalData[0] = (void *)typeName;
    internalData[1] = (void *)loader;
}

const char *JClass::getTypeName(void) const {
    void **internalData = (void **)&getFields()[1];
    return (char *)internalData[0];
}

ClassLoader *JClass::getClassLoader(void) const {
    void **internalData = (void **)&getFields()[1];
    return (ClassLoader *)internalData[1];
}

char JClass::isPrimitive(const char *typeName, uint16_t length) {
    switch(Hash(typeName, length)) {
        case Hash("int"): return strncmp(typeName, "int", length) == 0 ? 'I' : 0;
        case Hash("void"): return strncmp(typeName, "void", length) == 0 ? 'V' : 0;
        case Hash("byte"): return strncmp(typeName, "byte", length) == 0 ? 'B' : 0;
        case Hash("char"): return strncmp(typeName, "char", length) == 0 ? 'C' : 0;
        case Hash("long"): return strncmp(typeName, "long", length) == 0 ? 'J' : 0;
        case Hash("float"): return strncmp(typeName, "float", length) == 0 ? 'F' : 0;
        case Hash("short"): return strncmp(typeName, "short", length) == 0 ? 'S' : 0;
        case Hash("double"): return strncmp(typeName, "double", length) == 0 ? 'D' : 0;
        case Hash("boolean"): return strncmp(typeName, "boolean", length) == 0 ? 'Z' : 0;
        default: return 0;
    }
}

bool JClass::isPrimitive(void) const {
    return getClassLoader() == NULL;
}

bool JClass::isArray(void) const {
    const char *typeName = getTypeName();
    return typeName[0] == '[';
}

uint8_t JClass::componentSize() const {
    const char *typeName = getTypeName();
    if(typeName[0] != '[')
        return 0;
    switch(typeName[1]) {
        case 'Z':
        case 'B':
            return 1;
        case 'C':
        case 'S':
            return 2;
        case 'J':
        case 'D':
            return 8;
        default:
            return 4;
    }
}

uint32_t JClass::size(void) {
    return sizeof(JClass) + sizeof(FieldsData) + sizeof(void *) * 2;
}
