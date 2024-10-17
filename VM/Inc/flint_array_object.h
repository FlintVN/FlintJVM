
#ifndef __FLINT_ARRAY_OBJECT_H
#define __FLINT_ARRAY_OBJECT_H

#include "flint_object.h"

class FlintInt8Array : public FlintObject {
private:
    FlintInt8Array(void) = delete;
    FlintInt8Array(const FlintInt8Array &) = delete;
    void operator=(const FlintInt8Array &) = delete;

    using FlintObject::size;
    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    int8_t *getData(void) const;
};

class FlintInt16Array : public FlintObject {
private:
    FlintInt16Array(void) = delete;
    FlintInt16Array(const FlintInt16Array &) = delete;
    void operator=(const FlintInt16Array &) = delete;

    using FlintObject::size;
    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    int16_t *getData(void) const;
};

class FlintInt32Array : public FlintObject {
private:
    FlintInt32Array(void) = delete;
    FlintInt32Array(const FlintInt32Array &) = delete;
    void operator=(const FlintInt32Array &) = delete;

    using FlintObject::size;
    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    int32_t *getData(void) const;
};

class FlintFloatArray : public FlintObject {
private:
    FlintFloatArray(void) = delete;
    FlintFloatArray(const FlintFloatArray &) = delete;
    void operator=(const FlintFloatArray &) = delete;

    using FlintObject::size;
    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    float *getData(void) const;
};

class FlintInt64Array : public FlintObject {
private:
    FlintInt64Array(void) = delete;
    FlintInt64Array(const FlintInt64Array &) = delete;
    void operator=(const FlintInt64Array &) = delete;

    using FlintObject::size;
    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    int64_t *getData(void) const;
};

class FlintDoubleArray : public FlintObject {
private:
    FlintDoubleArray(void) = delete;
    FlintDoubleArray(const FlintDoubleArray &) = delete;
    void operator=(const FlintDoubleArray &) = delete;

    using FlintObject::size;
    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    double *getData(void) const;
};

class FlintObjectArray : public FlintObject {
private:
    FlintObjectArray(void) = delete;
    FlintObjectArray(const FlintObjectArray &) = delete;
    void operator=(const FlintObjectArray &) = delete;

    using FlintObject::getFields;
public:
    uint32_t getLength(void) const;
    FlintObject **getData(void) const;
};

#endif /* __FLINT_ARRAY_OBJECT_H */
