
#include "flint.h"
#include "flint_native_reflection.h"

jclass NativeReflection_GetCallerClass(FNIEnv *env) {
    return ((FExec *)env)->getCallerClass();
}

jint NativeReflection_GetClassAccessFlags(FNIEnv *env, jclass cls) {
    (void)env;
    return cls->getClassLoader()->getAccessFlag();
}

jbool NativeReflection_AreNestMates(FNIEnv *env, jclass currentClass, jclass memberClass) {
    jclass currentNestHost = currentClass->getNestHost((FExec *)env);
    jclass memberNestHost = memberClass->getNestHost((FExec *)env);
    if(currentNestHost == NULL || memberNestHost == NULL) return false;
    return currentNestHost == memberNestHost;
}
