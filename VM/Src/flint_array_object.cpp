
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
