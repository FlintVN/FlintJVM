
#include "flint.h"
#include "flint_native_string.h"

jstring nativeIntern(FNIEnv *env, jstring obj) {
    return Flint::getConstString(env->exec, obj);
}
