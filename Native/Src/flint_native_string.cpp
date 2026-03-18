
#include "flint.h"
#include "flint_native_string.h"

jstring NativeString_Intern(FNIEnv *env, jstring obj) {
    return env->getFlint()->getConstString(env->exec, obj);
}
