
#include <string.h>
#include "flint_array_object.h"

uint32_t FlintInt8Array::getLength(void) const {
    return size;
}

int8_t *FlintInt8Array::getData(void) const {
    return (int8_t *)data;
}

void FlintInt8Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t FlintInt16Array::getLength(void) const {
    return size / sizeof(int16_t);
}

int16_t *FlintInt16Array::getData(void) const {
    return (int16_t *)data;
}

void FlintInt16Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t FlintInt32Array::getLength(void) const {
    return size / sizeof(uint32_t);
}

int32_t *FlintInt32Array::getData(void) const {
    return (int32_t *)data;
}

void FlintInt32Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t FlintFloatArray::getLength(void) const {
    return size / sizeof(float);
}

float *FlintFloatArray::getData(void) const {
    return (float *)data;
}

void FlintFloatArray::clearData(void) {
    memset(data, 0, size);
}

uint32_t FlintInt64Array::getLength(void) const {
    return size / sizeof(int64_t);
}

int64_t *FlintInt64Array::getData(void) const {
    return (int64_t *)data;
}

void FlintInt64Array::clearData(void) {
    memset(data, 0, size);
}

uint32_t FlintDoubleArray::getLength(void) const {
    return size / sizeof(double);
}

double *FlintDoubleArray::getData(void) const {
    return (double *)data;
}

void FlintDoubleArray::clearData(void) {
    memset(data, 0, size);
}

uint32_t FlintObjectArray::getLength(void) const {
    return size / sizeof(FlintObject);
}

FlintObject **FlintObjectArray::getData(void) const {
    return (FlintObject **)data;
}

void FlintObjectArray::clearData(void) {
    memset(data, 0, size);
}
