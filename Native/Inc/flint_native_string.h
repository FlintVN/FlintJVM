
#ifndef __FLINT_NATIVE_STRING_H
#define __FLINT_NATIVE_STRING_H

#include "flint_native.h"

jstring NativeString_Intern(FNIEnv *env, jstring str);

static constexpr NativeMethod stringMethods[] = {
    NATIVE_METHOD("intern", "()Ljava/lang/String;", NativeString_Intern),
};

#endif /* __FLINT_NATIVE_STRING_H */
