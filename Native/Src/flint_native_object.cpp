
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_native_object.h"

jclass NativeObject_GetClass(FNIEnv *env, jobject obj) {
    return (obj->type != NULL) ? obj->type : Flint::getClassOfClass(env->exec);
}

jint NativeObject_HashCode(FNIEnv *env, jobject obj) {
    (void)env;
    return (int32_t)obj;
}

jobject NativeObject_Clone(FNIEnv *env, jobject obj) {
    if(obj->isArray()) {
        uint32_t count = ((jarray)obj)->getLength();
        jarray cloneObj = (jarray)Flint::newArray(env->exec, obj->type, count);
        if(cloneObj == NULL) return NULL;
        memcpy(cloneObj->getData(), ((jarray)obj)->getData(), ((jarray)obj)->getSizeInByte());
        return cloneObj;
    }
    else {
        env->throwNew(env->findClass("java/lang/CloneNotSupportedException"), "Clone method is not supported");
        return NULL;
    }
}
