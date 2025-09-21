
#ifndef __FLINT_NATIVE_INTERFACE_H
#define __FLINT_NATIVE_INTERFACE_H

#include "flint_std.h"

typedef int32_t                     jbool;
typedef int8_t                      jbyte;
typedef uint16_t                    jchar;
typedef int16_t                     jshort;
typedef int32_t                     jint;
typedef float                       jfloat;
typedef void                        jvoid;

typedef class JObject               *jobject;
typedef class JString               *jstring;
typedef class JClass                *jclass;
typedef class JThread               *jthread;
typedef class JThrowable            *jthrowable;
typedef class MethodInfo            *jmethodId;

typedef class JArray                *jarray;
typedef class JInt8Array            *jboolArray;
typedef class JUInt16Array          *jcharArray;
typedef class JInt8Array            *jbyteArray;
typedef class JInt16Array           *jshortArray;
typedef class JInt32Array           *jintArray;
typedef class JInt64Array           *jlongArray;
typedef class JFloatArray           *jfloatArray;
typedef class JDoubleArray          *jdoubleArray;
typedef class JObjectArray          *jobjectArray;

class jlong {
private:
    uint32_t low;
    uint32_t high;
public:
    jlong(int64_t value) : low(((uint32_t *)&value)[0]), high(((uint32_t *)&value)[1]) { }

    operator int64_t() const;
    void operator=(int64_t value);
};

class jdouble {
private:
    uint32_t low;
    uint32_t high;
public:
    jdouble(double value) : low(((uint32_t *)&value)[0]), high(((uint32_t *)&value)[1]) { }

    operator double() const;
    void operator=(double value);
};

class FNIEnv {
public:
    virtual jvoid print(const char *txt) const;
    virtual jvoid print(jstring str) const;

    virtual jclass findClass(const char *name, uint16_t length = 0xFFFF);

    virtual jbool isInstanceof(jobject obj, jclass type);
    virtual jbool isAssignableFrom(jclass fromType, jclass toType);

    virtual jobject newObject(jclass type);
    virtual jstring newString(const char *format, ...);
    virtual jboolArray newBoolArray(uint32_t count);
    virtual jbyteArray newByteArray(uint32_t count);
    virtual jcharArray newCharArray(uint32_t count);
    virtual jshortArray newShortArray(uint32_t count);
    virtual jintArray newIntArray(uint32_t count);
    virtual jlongArray newLongArray(uint32_t count);
    virtual jfloatArray newFloatArray(uint32_t count);
    virtual jdoubleArray newDoubleArray(uint32_t count);
    virtual jobjectArray newObjectArray(jclass type, uint32_t count);

    virtual jvoid throwNew(jclass cls, const char *msg = NULL, ...);

public: /* Do not publicize the functions below to users */
    jvoid freeObject(jobject obj);
    class FExec * const exec;
private:
    FNIEnv(class FExec *exec);
    FNIEnv(const FNIEnv &) = delete;
    void operator=(const FNIEnv &) = delete;

    friend class FExec;
};

#endif /* __FLINT_NATIVE_INTERFACE_H */
