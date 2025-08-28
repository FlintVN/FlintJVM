
#ifndef __FLINT_NATIVE_STRING_H
#define __FLINT_NATIVE_STRING_H

#include "flint_native.h"

jstring nativeIntern(FNIEnv *env, jstring str);

static constexpr NativeMethod stringMethods[] = {
    NATIVE_METHOD("intern", "()Ljava/lang/String;", nativeIntern),
};

#endif /* __FLINT_NATIVE_STRING_H */
