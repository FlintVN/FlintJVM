
#ifndef __FLINT_NATIVE_INFLATER_H
#define __FLINT_NATIVE_INFLATER_H

#include "flint_native.h"

jint NativeInflater_Init(FNIEnv *env, jbool nowrap);
jvoid NativeInflater_SetDictionary(FNIEnv *env, jint addr, jbyteArray b, jint off, jint len);
jint NativeInflater_InflateBytes(FNIEnv *env, jobject obj, jint addr, jbyteArray b, jint off, jint len);
jint NativeInflater_GetAdler(FNIEnv *env, jint addr);
jvoid NativeInflater_Reset(FNIEnv *env, jint addr);
jvoid NativeInflater_End(FNIEnv *env, jint addr);

inline constexpr NativeMethod inflaterMethods[] = {
    NATIVE_METHOD("init",          "(Z)I",     NativeInflater_Init),
    NATIVE_METHOD("setDictionary", "(I[BII)V", NativeInflater_SetDictionary),
    NATIVE_METHOD("inflateBytes",  "(I[BII)I", NativeInflater_InflateBytes),
    NATIVE_METHOD("getAdler",      "(I)I",     NativeInflater_GetAdler),
    NATIVE_METHOD("reset",         "(I)V",     NativeInflater_Reset),
    NATIVE_METHOD("end",           "(I)V",     NativeInflater_End),
};

#endif /* __FLINT_NATIVE_INFLATER_H */
