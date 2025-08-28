
#ifndef __FLINT_NATIVE_PRINT_STREAM_H
#define __FLINT_NATIVE_PRINT_STREAM_H

#include "flint_native.h"

void nativeWrite(FNIEnv *env, jstring str);
void nativeWriteln(FNIEnv *env, jstring str);

static constexpr NativeMethod printStreamMethods[] = {
    NATIVE_METHOD("write",   "(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("writeln", "(Ljava/lang/String;)V", nativeWriteln),
};

#endif /* __FLINT_NATIVE_PRINT_STREAM_H */
