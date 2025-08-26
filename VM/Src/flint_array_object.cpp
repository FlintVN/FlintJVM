
#include <string.h>
#include "flint_array_object.h"

uint32_t JInt8Array::getLength(void) const {
    return size;
}

int8_t *JInt8Array::getData(void) const {
    return (int8_t *)data;
}

void JInt8Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t JInt16Array::getLength(void) const {
    return size / sizeof(int16_t);
}

int16_t *JInt16Array::getData(void) const {
    return (int16_t *)data;
}

void JInt16Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t JInt32Array::getLength(void) const {
    return size / sizeof(uint32_t);
}

int32_t *JInt32Array::getData(void) const {
    return (int32_t *)data;
}

void JInt32Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t JFloatArray::getLength(void) const {
    return size / sizeof(float);
}

float *JFloatArray::getData(void) const {
    return (float *)data;
}

void JFloatArray::clearData(void) {
    memset(data, 0, size);
}

uint32_t JInt64Array::getLength(void) const {
    return size / sizeof(int64_t);
}

int64_t *JInt64Array::getData(void) const {
    return (int64_t *)data;
}

void JInt64Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t JDoubleArray::getLength(void) const {
    return size / sizeof(double);
}

double *JDoubleArray::getData(void) const {
    return (double *)data;
}

void JDoubleArray::clearData(void) {
    memset(data, 0, size);
}

uint32_t JObjectArray::getLength(void) const {
    return size / sizeof(JObject);
}

JObject **JObjectArray::getData(void) const {
    return (JObject **)data;
}

void JObjectArray::clearData(void) {
    memset(data, 0, size);
}
