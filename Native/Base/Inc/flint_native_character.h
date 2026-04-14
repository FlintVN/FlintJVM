
#ifndef __FLINT_NATIVE_CHARACTER_H
#define __FLINT_NATIVE_CHARACTER_H

#include "flint_native.h"

jint NativeCharacter_ToLowerCase(FNIEnv *env, jchar c);
jint NativeCharacter_ToUpperCase(FNIEnv *env, jchar c);

inline constexpr NativeMethod characterMethods[] = {
    NATIVE_METHOD("toLowerCase", "(C)C", NativeCharacter_ToLowerCase),
    NATIVE_METHOD("toUpperCase", "(C)C", NativeCharacter_ToUpperCase),
};

#endif /* __FLINT_NATIVE_CHARACTER_H */
