
#ifndef __FLINT_NATIVE_INTERFACE_H
#define __FLINT_NATIVE_INTERFACE_H

#include <cstdarg>
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
typedef class FieldValue            *jfieldId;
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
    virtual jclass findClass(const char *name, uint16_t length = 0xFFFF) = 0;

    virtual jbool isInstanceof(jobject obj, jclass type) = 0;
    virtual jbool isAssignableFrom(jclass fromType, jclass toType) = 0;

    virtual jobject newObject(jclass type) = 0;
    virtual jobject newObject(jclass type, jmethodId ctor, ...) = 0;
    virtual jstring newString(const char *format, ...) = 0;
    virtual jboolArray newBoolArray(uint32_t count) = 0;
    virtual jbyteArray newByteArray(uint32_t count) = 0;
    virtual jcharArray newCharArray(uint32_t count) = 0;
    virtual jshortArray newShortArray(uint32_t count) = 0;
    virtual jintArray newIntArray(uint32_t count) = 0;
    virtual jlongArray newLongArray(uint32_t count) = 0;
    virtual jfloatArray newFloatArray(uint32_t count) = 0;
    virtual jdoubleArray newDoubleArray(uint32_t count) = 0;
    virtual jobjectArray newObjectArray(jclass type, uint32_t count) = 0;

    virtual jfieldId getFieldId(jobject obj, const char *name) = 0;
    virtual jbool getBoolField(jfieldId fid) = 0;
    virtual jbyte getByteField(jfieldId fid) = 0;
    virtual jchar getCharField(jfieldId fid) = 0;
    virtual jshort getShortField(jfieldId fid) = 0;
    virtual jint getIntField(jfieldId fid) = 0;
    virtual jfloat getFloatField(jfieldId fid) = 0;
    virtual jlong getLongField(jfieldId fid) = 0;
    virtual jdouble getDoubleField(jfieldId fid) = 0;
    virtual jobject getObjField(jfieldId fid) = 0;

    virtual jvoid setBoolField(jfieldId fid, jbool val) = 0;
    virtual jvoid setByteField(jfieldId fid, jbyte val) = 0;
    virtual jvoid setCharField(jfieldId fid, jchar val) = 0;
    virtual jvoid setShortField(jfieldId fid, jshort val) = 0;
    virtual jvoid setIntField(jfieldId fid, jint val) = 0;
    virtual jvoid setFloatField(jfieldId fid, jfloat val) = 0;
    virtual jvoid setLongField(jfieldId fid, jlong val) = 0;
    virtual jvoid setDoubleField(jfieldId fid, jdouble val) = 0;
    virtual jvoid setObjField(jfieldId fid, jobject obj) = 0;

    virtual jmethodId getMethodId(jclass cls, const char *name, const char *sig) = 0;
    virtual jmethodId getConstructorId(jclass cls, const char *sig) = 0;
    virtual jvoid callVoidMethod(jmethodId mtid, ...) = 0;
    virtual jbool callBoolMethod(jmethodId mtid, ...) = 0;
    virtual jbyte callByteMethod(jmethodId mtid, ...) = 0;
    virtual jchar callCharMethod(jmethodId mtid, ...) = 0;
    virtual jshort callShortMethod(jmethodId mtid, ...) = 0;
    virtual jint callIntMethod(jmethodId mtid, ...) = 0;
    virtual jlong callLongMethod(jmethodId mtid, ...) = 0;
    virtual jfloat callFloatMethod(jmethodId mtid, ...) = 0;
    virtual jdouble callDoubleMethod(jmethodId mtid, ...) = 0;
    virtual jobject callObjectMethod(jmethodId mtid, ...) = 0;

    virtual jvoid throwNew(jclass cls, const char *msg = NULL, ...) = 0;
    virtual jbool hasTerminateRequest(void) = 0;

    virtual jvoid freeObject(jobject obj) = 0;
protected:
    FNIEnv(void);
private:
    FNIEnv(const FNIEnv &) = delete;
    void operator=(const FNIEnv &) = delete;
};

#endif /* __FLINT_NATIVE_INTERFACE_H */
