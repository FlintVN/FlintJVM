
#ifndef __FLINT_NATIVE_CHARACTER_H
#define __FLINT_NATIVE_CHARACTER_H

#include "flint_native.h"

jint nativeToLowerCase(FNIEnv *env, jchar c);
jint nativeToUpperCase(FNIEnv *env, jchar c);

static constexpr NativeMethod characterMethods[] = {
    NATIVE_METHOD("toLowerCase", "(C)C", nativeToLowerCase),
    NATIVE_METHOD("toUpperCase", "(C)C", nativeToUpperCase),
};

#endif /* __FLINT_NATIVE_CHARACTER_H */
