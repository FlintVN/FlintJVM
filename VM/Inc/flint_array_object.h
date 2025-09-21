
#ifndef __FLINT_ARRAY_OBJECT_H
#define __FLINT_ARRAY_OBJECT_H

#include "flint_java_object.h"

class JArray : public JObject {
public:
    uint32_t getLength(void) const;
    uint32_t getSizeInByte(void) const;
    uint8_t componentSize() const;
    const char *getCompTypeName(uint16_t *length);
    void *getData(void) const;
    void clearArray(void);
private:
    JArray(void) = delete;
    JArray(const JArray &) = delete;
    void operator=(const JArray &) = delete;
};

class JInt8Array : public JArray {
public:
    uint32_t getLength(void) const;
    int8_t *getData(void) const;
private:
    JInt8Array(void) = delete;
    JInt8Array(const JInt8Array &) = delete;
    void operator=(const JInt8Array &) = delete;
};

class JInt16Array : public JArray {
public:
    uint32_t getLength(void) const;
    int16_t *getData(void) const;
private:
    JInt16Array(void) = delete;
    JInt16Array(const JInt16Array &) = delete;
    void operator=(const JInt16Array &) = delete;
};

class JUInt16Array : public JArray {
public:
    uint32_t getLength(void) const;
    uint16_t *getData(void) const;
private:
    JUInt16Array(void) = delete;
    JUInt16Array(const JUInt16Array &) = delete;
    void operator=(const JUInt16Array &) = delete;
};

class JInt32Array : public JArray {
public:
    uint32_t getLength(void) const;
    int32_t *getData(void) const;
private:
    JInt32Array(void) = delete;
    JInt32Array(const JInt32Array &) = delete;
    void operator=(const JInt32Array &) = delete;
};

class JFloatArray : public JArray {
public:
    uint32_t getLength(void) const;
    float *getData(void) const;
private:
    JFloatArray(void) = delete;
    JFloatArray(const JFloatArray &) = delete;
    void operator=(const JFloatArray &) = delete;
};

class JInt64Array : public JArray {
public:
    uint32_t getLength(void) const;
    int64_t *getData(void) const;
private:
    JInt64Array(void) = delete;
    JInt64Array(const JInt64Array &) = delete;
    void operator=(const JInt64Array &) = delete;
};

class JDoubleArray : public JArray {
public:
    uint32_t getLength(void) const;
    double *getData(void) const;
private:
    JDoubleArray(void) = delete;
    JDoubleArray(const JDoubleArray &) = delete;
    void operator=(const JDoubleArray &) = delete;
};

class JObjectArray : public JArray {
public:
    uint32_t getLength(void) const;
    JObject **getData(void) const;
private:
    JObjectArray(void) = delete;
    JObjectArray(const JObjectArray &) = delete;
    void operator=(const JObjectArray &) = delete;
};

typedef JInt8Array      JBoolArray;
typedef JInt16Array     JCharArray;
typedef JInt8Array      JByteArray;
typedef JInt16Array     JShortArray;
typedef JInt32Array     JIntArray;
typedef JInt64Array     JLongArray;

#endif /* __FLINT_ARRAY_OBJECT_H */
