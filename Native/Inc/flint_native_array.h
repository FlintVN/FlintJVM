
#ifndef __FLINT_NATIVE_ARRAY_H
#define __FLINT_NATIVE_ARRAY_H

#include "flint_native.h"

jint nativeArrayGetLength(FNIEnv *env, jobject obj);
jobject nativeArrayGet(FNIEnv *env, jobject obj, jint index);
jbool nativeArrayGetBoolean(FNIEnv *env, jobject obj, jint index);
jbyte nativeArrayGetByte(FNIEnv *env, jobject obj, jint index);
jchar nativeArrayGetChar(FNIEnv *env, jobject obj, jint index);
jshort nativeArrayGetShort(FNIEnv *env, jobject obj, jint index);
jint nativeArrayGetInt(FNIEnv *env, jobject obj, jint index);
jlong nativeArrayGetLong(FNIEnv *env, jobject obj, jint index);
jfloat nativeArrayGetFloat(FNIEnv *env, jobject obj, jint index);
jdouble nativeArrayGetDouble(FNIEnv *env, jobject obj, jint index);
jvoid nativeArraySet(FNIEnv *env, jobject obj, jint index, jobject v);
jvoid nativeArraySetBoolean(FNIEnv *env, jobject obj, jint index, jbool v);
jvoid nativeArraySetByte(FNIEnv *env, jobject obj, jint index, jbyte v);
jvoid nativeArraySetChar(FNIEnv *env, jobject obj, jint index, jchar v);
jvoid nativeArraySetShort(FNIEnv *env, jobject obj, jint index, jshort v);
jvoid nativeArraySetInt(FNIEnv *env, jobject obj, jint index, jint v);
jvoid nativeArraySetLong(FNIEnv *env, jobject obj, jint index, jlong v);
jvoid nativeArraySetFloat(FNIEnv *env, jobject obj, jint index, jfloat v);
jvoid nativeArraySetDouble(FNIEnv *env, jobject obj, jint index, jdouble v);
jobject nativeArrayNewArray(FNIEnv *env, jclass componentType, jint length);
jobject nativeArrayMultiNewArray(FNIEnv *env, jclass componentType, jintArray dimensions);

static constexpr NativeMethod arrayMethods[] = {
    NATIVE_METHOD("getLength",     "(Ljava/lang/Object;)I",                    nativeArrayGetLength),
    NATIVE_METHOD("get",           "(Ljava/lang/Object;I)Ljava/lang/Object;",  nativeArrayGet),
    NATIVE_METHOD("getBoolean",    "(Ljava/lang/Object;I)Z",                   nativeArrayGetBoolean),
    NATIVE_METHOD("getByte",       "(Ljava/lang/Object;I)B",                   nativeArrayGetByte),
    NATIVE_METHOD("getChar",       "(Ljava/lang/Object;I)C",                   nativeArrayGetChar),
    NATIVE_METHOD("getShort",      "(Ljava/lang/Object;I)S",                   nativeArrayGetShort),
    NATIVE_METHOD("getInt",        "(Ljava/lang/Object;I)I",                   nativeArrayGetInt),
    NATIVE_METHOD("getLong",       "(Ljava/lang/Object;I)J",                   nativeArrayGetLong),
    NATIVE_METHOD("getFloat",      "(Ljava/lang/Object;I)F",                   nativeArrayGetFloat),
    NATIVE_METHOD("getDouble",     "(Ljava/lang/Object;I)D",                   nativeArrayGetDouble),
    NATIVE_METHOD("set",           "(Ljava/lang/Object;ILjava/lang/Object;)V", nativeArraySet),
    NATIVE_METHOD("setBoolean",    "(Ljava/lang/Object;IZ)V",                  nativeArraySetBoolean),
    NATIVE_METHOD("setByte",       "(Ljava/lang/Object;IB)V",                  nativeArraySetByte),
    NATIVE_METHOD("setChar",       "(Ljava/lang/Object;IC)V",                  nativeArraySetChar),
    NATIVE_METHOD("setShort",      "(Ljava/lang/Object;IS)V",                  nativeArraySetShort),
    NATIVE_METHOD("setInt",        "(Ljava/lang/Object;II)V",                  nativeArraySetInt),
    NATIVE_METHOD("setLong",       "(Ljava/lang/Object;IJ)V",                  nativeArraySetLong),
    NATIVE_METHOD("setFloat",      "(Ljava/lang/Object;IF)V",                  nativeArraySetFloat),
    NATIVE_METHOD("setDouble",     "(Ljava/lang/Object;ID)V",                  nativeArraySetDouble),
    NATIVE_METHOD("newArray",      "(Ljava/lang/Class;I)Ljava/lang/Object;",   nativeArrayNewArray),
    NATIVE_METHOD("multiNewArray", "(Ljava/lang/Class;[I)Ljava/lang/Object;",  nativeArrayMultiNewArray),
};

#endif /* __FLINT_NATIVE_ARRAY_H */
