
#include <string.h>
#include "flint.h"
#include "flint_common.h"
#include "flint_java_class.h"
#include "flint_array_object.h"
#include "flint_native_method.h"

jobject NativeConstructor_NewInstance0(FNIEnv *env, jobject obj, jobjectArray initargs) {
    jmethodId methodInfo = (jmethodId)obj->getFieldByIndex(0)->getInt32();  /* entry */
    jclass cls = (jclass)obj->getFieldByIndex(1)->getObj();
    jobjectArray ptypes = (jobjectArray)obj->getFieldByIndex(2)->getObj();
    FExec *exec = env->exec;

    jobject newObj = env->newObject(cls);
    if(newObj == NULL) return NULL;
    exec->stackPushObject(newObj);

    int32_t argSlot = 1;
    int32_t argc = ptypes->getLength();
    for(uint32_t i = 0; i < argc; i++) {
        jclass ptype = (jclass)ptypes->getData()[i];
        if(ptype->isPrimitive()) {
            const char *ptypeName = ptype->getTypeName();
            if(strcmp(ptypeName, "long") == 0 || strcmp(ptypeName, "double") == 0) {
                exec->stackPushInt64(initargs->getData()[i]->getFieldByIndex(0)->getInt64());
                argSlot += 2;
            }
            else {
                exec->stackPushInt32(initargs->getData()[i]->getFieldByIndex(0)->getInt32());
                argSlot++;
            }
        }
        else {
            exec->stackPushObject(initargs->getData()[i]);
            argSlot++;
        }
    }

    exec->callMethod(methodInfo, argSlot);

    if(exec->hasTerminateRequest()) {
        Flint::freeObject(newObj);
        return NULL;
    }
    if(exec->hasException()) {
        env->throwNew(env->findClass("java/lang/reflect/InvocationTargetException"));
        return NULL;
    }

    return newObj;
}
