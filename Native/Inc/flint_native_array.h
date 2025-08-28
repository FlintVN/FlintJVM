
#ifndef __FLINT_NATIVE_ARRAY_H
#define __FLINT_NATIVE_ARRAY_H

#include "flint_native.h"

jint nativeGetLength(FNIEnv *env, jobject obj);
jobject nativeGet(FNIEnv *env, jobject obj, jint index);
jbool nativeGetBoolean(FNIEnv *env, jobject obj, jint index);
jbyte nativeGetByte(FNIEnv *env, jobject obj, jint index);
jchar nativeGetChar(FNIEnv *env, jobject obj, jint index);
jshort nativeGetShort(FNIEnv *env, jobject obj, jint index);
jint nativeGetInt(FNIEnv *env, jobject obj, jint index);
jlong nativeGetLong(FNIEnv *env, jobject obj, jint index);
jfloat nativeGetFloat(FNIEnv *env, jobject obj, jint index);
jdouble nativeGetDouble(FNIEnv *env, jobject obj, jint index);
jvoid nativeSet(FNIEnv *env, jobject obj, jint index, jobject v);
jvoid nativeSetBoolean(FNIEnv *env, jobject obj, jint index, jbool v);
jvoid nativeSetByte(FNIEnv *env, jobject obj, jint index, jbyte v);
jvoid nativeSetChar(FNIEnv *env, jobject obj, jint index, jchar v);
jvoid nativeSetShort(FNIEnv *env, jobject obj, jint index, jshort v);
jvoid nativeSetInt(FNIEnv *env, jobject obj, jint index, jint v);
jvoid nativeSetLong(FNIEnv *env, jobject obj, jint index, jlong v);
jvoid nativeSetFloat(FNIEnv *env, jobject obj, jint index, jfloat v);
jvoid nativeSetDouble(FNIEnv *env, jobject obj, jint index, jdouble v);
jobject nativeNewArray(FNIEnv *env, jclass componentType, jint length);
jobject nativeMultiNewArray(FNIEnv *env, jclass componentType, jintArray dimensions);

static constexpr NativeMethod arrayMethods[] = {
    NATIVE_METHOD("getLength",     "(Ljava/lang/Object;)I",                    nativeGetLength),
    NATIVE_METHOD("get",           "(Ljava/lang/Object;I)Ljava/lang/Object;",  nativeGet),
    NATIVE_METHOD("getBoolean",    "(Ljava/lang/Object;I)Z",                   nativeGetBoolean),
    NATIVE_METHOD("getByte",       "(Ljava/lang/Object;I)B",                   nativeGetByte),
    NATIVE_METHOD("getChar",       "(Ljava/lang/Object;I)C",                   nativeGetChar),
    NATIVE_METHOD("getShort",      "(Ljava/lang/Object;I)S",                   nativeGetShort),
    NATIVE_METHOD("getInt",        "(Ljava/lang/Object;I)I",                   nativeGetInt),
    NATIVE_METHOD("getLong",       "(Ljava/lang/Object;I)J",                   nativeGetLong),
    NATIVE_METHOD("getFloat",      "(Ljava/lang/Object;I)F",                   nativeGetFloat),
    NATIVE_METHOD("getDouble",     "(Ljava/lang/Object;I)D",                   nativeGetDouble),
    NATIVE_METHOD("set",           "(Ljava/lang/Object;ILjava/lang/Object;)V", nativeSet),
    NATIVE_METHOD("setBoolean",    "(Ljava/lang/Object;IZ)V",                  nativeSetBoolean),
    NATIVE_METHOD("setByte",       "(Ljava/lang/Object;IB)V",                  nativeSetByte),
    NATIVE_METHOD("setChar",       "(Ljava/lang/Object;IC)V",                  nativeSetChar),
    NATIVE_METHOD("setShort",      "(Ljava/lang/Object;IS)V",                  nativeSetShort),
    NATIVE_METHOD("setInt",        "(Ljava/lang/Object;II)V",                  nativeSetInt),
    NATIVE_METHOD("setLong",       "(Ljava/lang/Object;IJ)V",                  nativeSetLong),
    NATIVE_METHOD("setFloat",      "(Ljava/lang/Object;IF)V",                  nativeSetFloat),
    NATIVE_METHOD("setDouble",     "(Ljava/lang/Object;ID)V",                  nativeSetDouble),
    NATIVE_METHOD("newArray",      "(Ljava/lang/Class;I)Ljava/lang/Object;",   nativeNewArray),
    NATIVE_METHOD("multiNewArray", "(Ljava/lang/Class;[I)Ljava/lang/Object;",  nativeMultiNewArray),
};

#endif /* __FLINT_NATIVE_ARRAY_H */
