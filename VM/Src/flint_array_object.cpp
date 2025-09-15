
#include <string.h>
#include "flint_java_class.h"
#include "flint_array_object.h"

uint32_t JArray::getLength(void) const {
    return size / type->componentSize();
}

uint32_t JArray::getSizeInByte(void) const {
    return size;
}

uint8_t JArray::componentSize() const {
    return type->componentSize();
}

const char *JArray::getCompTypeName(uint16_t *length) {
    const char *name = getTypeName();
    uint32_t len = 0;
    while(*name == '[') name++;
    if(*name == 'L') name++;
    while(name[len] && name[len] != ';') len++;
    if(len == 1) switch (*name) {
        case 'Z': *length = strlen("boolean"); return "boolean";
        case 'C': *length = strlen("char"); return "char";
        case 'F': *length = strlen("float"); return "float";
        case 'D': *length = strlen("double"); return "double";
        case 'B': *length = strlen("byte"); return "byte";
        case 'S': *length = strlen("short"); return "short";
        case 'I': *length = strlen("int"); return "int";
        case 'J': *length = strlen("long"); return "long";
        case 'V': *length = strlen("void"); return "void";
        default: break;
    }
    *length = len;
    return name;
}

void *JArray::getData(void) const {
    return (void *)data;
}

void JArray::clearArray(void) {
    clearData();
}

uint32_t JInt8Array::getLength(void) const {
    return size;
}

int8_t *JInt8Array::getData(void) const {
    return (int8_t *)data;
}

uint32_t JInt16Array::getLength(void) const {
    return size / sizeof(int16_t);
}

int16_t *JInt16Array::getData(void) const {
    return (int16_t *)data;
}

uint16_t *JUInt16Array::getData(void) const {
    return (uint16_t *)data;
}

uint32_t JInt32Array::getLength(void) const {
    return size / sizeof(uint32_t);
}

int32_t *JInt32Array::getData(void) const {
    return (int32_t *)data;
}

uint32_t JFloatArray::getLength(void) const {
    return size / sizeof(float);
}

float *JFloatArray::getData(void) const {
    return (float *)data;
}

uint32_t JInt64Array::getLength(void) const {
    return size / sizeof(int64_t);
}

int64_t *JInt64Array::getData(void) const {
    return (int64_t *)data;
}

uint32_t JDoubleArray::getLength(void) const {
    return size / sizeof(double);
}

double *JDoubleArray::getData(void) const {
    return (double *)data;
}

uint32_t JObjectArray::getLength(void) const {
    return size / sizeof(uint32_t);
}

JObject **JObjectArray::getData(void) const {
    return (JObject **)data;
}
