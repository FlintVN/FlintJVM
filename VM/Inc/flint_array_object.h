
#ifndef __FLINT_ARRAY_OBJECT_H
#define __FLINT_ARRAY_OBJECT_H

#include "flint_java_object.h"

class JInt8Array : public JObject {
private:
    JInt8Array(void) = delete;
    JInt8Array(const JInt8Array &) = delete;
    void operator=(const JInt8Array &) = delete;

    using JObject::size;
    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    int8_t *getData(void) const;
    void clearData(void);
};

class JInt16Array : public JObject {
private:
    JInt16Array(void) = delete;
    JInt16Array(const JInt16Array &) = delete;
    void operator=(const JInt16Array &) = delete;

    using JObject::size;
    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    int16_t *getData(void) const;
    void clearData(void);
};

class JInt32Array : public JObject {
private:
    JInt32Array(void) = delete;
    JInt32Array(const JInt32Array &) = delete;
    void operator=(const JInt32Array &) = delete;

    using JObject::size;
    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    int32_t *getData(void) const;
    void clearData(void);
};

class JFloatArray : public JObject {
private:
    JFloatArray(void) = delete;
    JFloatArray(const JFloatArray &) = delete;
    void operator=(const JFloatArray &) = delete;

    using JObject::size;
    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    float *getData(void) const;
    void clearData(void);
};

class JInt64Array : public JObject {
private:
    JInt64Array(void) = delete;
    JInt64Array(const JInt64Array &) = delete;
    void operator=(const JInt64Array &) = delete;

    using JObject::size;
    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    int64_t *getData(void) const;
    void clearData(void);
};

class JDoubleArray : public JObject {
private:
    JDoubleArray(void) = delete;
    JDoubleArray(const JDoubleArray &) = delete;
    void operator=(const JDoubleArray &) = delete;

    using JObject::size;
    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    double *getData(void) const;
    void clearData(void);
};

class JObjectArray : public JObject {
private:
    JObjectArray(void) = delete;
    JObjectArray(const JObjectArray &) = delete;
    void operator=(const JObjectArray &) = delete;

    using JObject::getFields;
public:
    uint32_t getLength(void) const;
    JObject **getData(void) const;
    void clearData(void);
};

#endif /* __FLINT_ARRAY_OBJECT_H */
