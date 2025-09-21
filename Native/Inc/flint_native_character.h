
#ifndef __FLINT_NATIVE_CHARACTER_H
#define __FLINT_NATIVE_CHARACTER_H

#include "flint_native.h"

jint nativeToLower(FNIEnv *env, jchar c);
jint nativeToUpper(FNIEnv *env, jchar c);

static constexpr NativeMethod characterMethods[] = {
    NATIVE_METHOD("toLower", "(C)C", nativeToLower),
    NATIVE_METHOD("toUpper", "(C)C", nativeToUpper),
};

#endif /* __FLINT_NATIVE_CHARACTER_H */
