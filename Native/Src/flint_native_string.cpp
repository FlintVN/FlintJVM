
#include "flint.h"
#include "flint_native_string.h"

jstring NativeString_Intern(FNIEnv *env, jstring obj) {
    return Flint::getConstString(env->exec, obj);
}
