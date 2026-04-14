
#ifndef __FLINT_NATIVE_ARRAY_H
#define __FLINT_NATIVE_ARRAY_H

#include "flint_native.h"

jint NativeArray_GetLength(FNIEnv *env, jobject obj);
jobject NativeArray_Get(FNIEnv *env, jobject obj, jint index);
jbool NativeArray_GetBoolean(FNIEnv *env, jobject obj, jint index);
jbyte NativeArray_GetByte(FNIEnv *env, jobject obj, jint index);
jchar NativeArray_GetChar(FNIEnv *env, jobject obj, jint index);
jshort NativeArray_GetShort(FNIEnv *env, jobject obj, jint index);
jint NativeArray_GetInt(FNIEnv *env, jobject obj, jint index);
jlong NativeArray_GetLong(FNIEnv *env, jobject obj, jint index);
jfloat NativeArray_GetFloat(FNIEnv *env, jobject obj, jint index);
jdouble NativeArray_GetDouble(FNIEnv *env, jobject obj, jint index);
jvoid NativeArray_Set(FNIEnv *env, jobject obj, jint index, jobject v);
jvoid NativeArray_SetBoolean(FNIEnv *env, jobject obj, jint index, jbool v);
jvoid NativeArray_SetByte(FNIEnv *env, jobject obj, jint index, jbyte v);
jvoid NativeArray_SetChar(FNIEnv *env, jobject obj, jint index, jchar v);
jvoid NativeArray_SetShort(FNIEnv *env, jobject obj, jint index, jshort v);
jvoid NativeArray_SetInt(FNIEnv *env, jobject obj, jint index, jint v);
jvoid NativeArray_SetLong(FNIEnv *env, jobject obj, jint index, jlong v);
jvoid NativeArray_SetFloat(FNIEnv *env, jobject obj, jint index, jfloat v);
jvoid NativeArray_SetDouble(FNIEnv *env, jobject obj, jint index, jdouble v);
jobject NativeArray_NewArray(FNIEnv *env, jclass componentType, jint length);
jobject NativeArray_MultiNewArray(FNIEnv *env, jclass componentType, jintArray dimensions);

inline constexpr NativeMethod arrayMethods[] = {
    NATIVE_METHOD("getLength",     "(Ljava/lang/Object;)I",                    NativeArray_GetLength),
    NATIVE_METHOD("get",           "(Ljava/lang/Object;I)Ljava/lang/Object;",  NativeArray_Get),
    NATIVE_METHOD("getBoolean",    "(Ljava/lang/Object;I)Z",                   NativeArray_GetBoolean),
    NATIVE_METHOD("getByte",       "(Ljava/lang/Object;I)B",                   NativeArray_GetByte),
    NATIVE_METHOD("getChar",       "(Ljava/lang/Object;I)C",                   NativeArray_GetChar),
    NATIVE_METHOD("getShort",      "(Ljava/lang/Object;I)S",                   NativeArray_GetShort),
    NATIVE_METHOD("getInt",        "(Ljava/lang/Object;I)I",                   NativeArray_GetInt),
    NATIVE_METHOD("getLong",       "(Ljava/lang/Object;I)J",                   NativeArray_GetLong),
    NATIVE_METHOD("getFloat",      "(Ljava/lang/Object;I)F",                   NativeArray_GetFloat),
    NATIVE_METHOD("getDouble",     "(Ljava/lang/Object;I)D",                   NativeArray_GetDouble),
    NATIVE_METHOD("set",           "(Ljava/lang/Object;ILjava/lang/Object;)V", NativeArray_Set),
    NATIVE_METHOD("setBoolean",    "(Ljava/lang/Object;IZ)V",                  NativeArray_SetBoolean),
    NATIVE_METHOD("setByte",       "(Ljava/lang/Object;IB)V",                  NativeArray_SetByte),
    NATIVE_METHOD("setChar",       "(Ljava/lang/Object;IC)V",                  NativeArray_SetChar),
    NATIVE_METHOD("setShort",      "(Ljava/lang/Object;IS)V",                  NativeArray_SetShort),
    NATIVE_METHOD("setInt",        "(Ljava/lang/Object;II)V",                  NativeArray_SetInt),
    NATIVE_METHOD("setLong",       "(Ljava/lang/Object;IJ)V",                  NativeArray_SetLong),
    NATIVE_METHOD("setFloat",      "(Ljava/lang/Object;IF)V",                  NativeArray_SetFloat),
    NATIVE_METHOD("setDouble",     "(Ljava/lang/Object;ID)V",                  NativeArray_SetDouble),
    NATIVE_METHOD("newArray",      "(Ljava/lang/Class;I)Ljava/lang/Object;",   NativeArray_NewArray),
    NATIVE_METHOD("multiNewArray", "(Ljava/lang/Class;[I)Ljava/lang/Object;",  NativeArray_MultiNewArray),
};

#endif /* __FLINT_NATIVE_ARRAY_H */
