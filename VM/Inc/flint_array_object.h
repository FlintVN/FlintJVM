
#ifndef __FLINT_ARRAY_OBJECT_H
#define __FLINT_ARRAY_OBJECT_H

#include "flint_java_object.h"

class FlintInt8Array : public FlintJavaObject {
private:
    FlintInt8Array(void) = delete;
    FlintInt8Array(const FlintInt8Array &) = delete;
    void operator=(const FlintInt8Array &) = delete;

    using FlintJavaObject::size;
    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    int8_t *getData(void) const;
    void clearData(void);
};

class FlintInt16Array : public FlintJavaObject {
private:
    FlintInt16Array(void) = delete;
    FlintInt16Array(const FlintInt16Array &) = delete;
    void operator=(const FlintInt16Array &) = delete;

    using FlintJavaObject::size;
    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    int16_t *getData(void) const;
    void clearData(void);
};

class FlintInt32Array : public FlintJavaObject {
private:
    FlintInt32Array(void) = delete;
    FlintInt32Array(const FlintInt32Array &) = delete;
    void operator=(const FlintInt32Array &) = delete;

    using FlintJavaObject::size;
    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    int32_t *getData(void) const;
    void clearData(void);
};

class FlintFloatArray : public FlintJavaObject {
private:
    FlintFloatArray(void) = delete;
    FlintFloatArray(const FlintFloatArray &) = delete;
    void operator=(const FlintFloatArray &) = delete;

    using FlintJavaObject::size;
    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    float *getData(void) const;
    void clearData(void);
};

class FlintInt64Array : public FlintJavaObject {
private:
    FlintInt64Array(void) = delete;
    FlintInt64Array(const FlintInt64Array &) = delete;
    void operator=(const FlintInt64Array &) = delete;

    using FlintJavaObject::size;
    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    int64_t *getData(void) const;
    void clearData(void);
};

class FlintDoubleArray : public FlintJavaObject {
private:
    FlintDoubleArray(void) = delete;
    FlintDoubleArray(const FlintDoubleArray &) = delete;
    void operator=(const FlintDoubleArray &) = delete;

    using FlintJavaObject::size;
    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    double *getData(void) const;
    void clearData(void);
};

class FlintObjectArray : public FlintJavaObject {
private:
    FlintObjectArray(void) = delete;
    FlintObjectArray(const FlintObjectArray &) = delete;
    void operator=(const FlintObjectArray &) = delete;

    using FlintJavaObject::getFields;
public:
    uint32_t getLength(void) const;
    FlintJavaObject **getData(void) const;
    void clearData(void);
};

#endif /* __FLINT_ARRAY_OBJECT_H */
