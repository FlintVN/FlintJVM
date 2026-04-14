
#include "flint.h"
#include "flint_native_string.h"

jstring NativeString_Intern(FNIEnv *env, jstring obj) {
    return ((FExec *)env)->getFlint()->getConstString((FExec *)env, obj);
}
