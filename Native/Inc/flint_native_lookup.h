
#ifndef __FLINT_NATIVE_LOOKUP_H
#define __FLINT_NATIVE_LOOKUP_H

#include "flint_native.h"

jobject nativeFindStatic(FNIEnv *env, jobject lookup, jclass refc, jstring name, jstring desc);
jobject nativeFindVirtual(FNIEnv *env, jobject lookup, jclass refc, jstring name, jstring desc);
jobject nativeFindConstructor(FNIEnv *env, jobject lookup, jclass refc, jstring desc);

static constexpr NativeMethod lookupMethods[] = {
    NATIVE_METHOD("findStatic",      "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/invoke/MethodHandle;",  nativeFindStatic),
    NATIVE_METHOD("findVirtual",     "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/invoke/MethodHandle;",  nativeFindVirtual),
    NATIVE_METHOD("findConstructor", "(Ljava/lang/Class;Ljava/lang/String;)Ljava/lang/invoke/MethodHandle;",                    nativeFindConstructor)
};

#endif /* __FLINT_NATIVE_LOOKUP_H */
