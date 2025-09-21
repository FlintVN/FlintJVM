
#include "flint_native_print_stream.h"

void nativeWrite(FNIEnv *env, jstring str) {
    if(str == NULL)
        env->print("null");
    else
        env->print(str);
}

void nativeWriteln(FNIEnv *env, jstring str) {
    if(str == NULL)
        env->print("null");
    else
        env->print(str);
    env->print("\n");
}
