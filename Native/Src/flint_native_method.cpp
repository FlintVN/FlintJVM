
#include <string.h>
#include "flint.h"
#include "flint_common.h"
#include "flint_java_class.h"
#include "flint_array_object.h"
#include "flint_native_method.h"

jobject NativeMethod_Invoke0(FNIEnv *env, jobject thisObj, jobject obj, jobjectArray args) {
    jmethodId methodInfo = (jmethodId)thisObj->getFieldByIndex(0)->getInt32();  /* entry */
    jclass rtype = (jclass)thisObj->getFieldByIndex(3)->getObj();
    jobjectArray ptypes = (jobjectArray)thisObj->getFieldByIndex(4)->getObj();
    FExec *exec = env->exec;

    int32_t argSlot = 0;
    int32_t argc = ptypes->getLength();
    if(!(methodInfo->accessFlag & METHOD_STATIC)) {
        exec->stackPushObject(obj);
        argSlot++;
    }
    for(uint32_t i = 0; i < argc; i++) {
        jclass ptype = (jclass)ptypes->getData()[i];
        if(ptype->isPrimitive()) {
            const char *ptypeName = ptype->getTypeName();
            if(strcmp(ptypeName, "long") == 0 || strcmp(ptypeName, "double") == 0) {
                exec->stackPushInt64(args->getData()[i]->getFieldByIndex(0)->getInt64());
                argSlot += 2;
            }
            else {
                exec->stackPushInt32(args->getData()[i]->getFieldByIndex(0)->getInt32());
                argSlot++;
            }
        }
        else {
            exec->stackPushObject(args->getData()[i]);
            argSlot++;
        }
    }

    uint64_t ret;
    if(methodInfo->accessFlag & METHOD_STATIC)
        ret = exec->callMethod(methodInfo, argSlot);
    else {
        JClass *objType = obj->type != NULL ? obj->type : Flint::getClassOfClass(exec);
        if(methodInfo->loader != objType->getClassLoader()) {
            methodInfo = Flint::findMethod(exec, objType, &methodInfo->nameAndType);
            if(methodInfo == NULL) return NULL;
            thisObj->getFieldByIndex(0)->setInt32((int32_t)methodInfo);
        }
        ret = exec->callMethod(methodInfo, argSlot);
    }
    if(exec->hasTerminateRequest())
        return NULL;
    if(exec->hasException()) {
        env->throwNew(env->findClass("java/lang/reflect/InvocationTargetException"));
        return NULL;
    }
    switch(rtype->getTypeName()[0]) {
        case 'B': { /* byte */
            jobject val = env->newObject(env->findClass("java/lang/Byte"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt32((int8_t)ret);
            return val;
        }
        case 'Z': { /* boolean */
            jobject val = env->newObject(env->findClass("java/lang/Boolean"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt32(!!ret);
            return val;
        }
        case 'C': { /* char */
            jobject val = env->newObject(env->findClass("java/lang/Character"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt32((uint16_t)ret);
            return val;
        }
        case 'S': { /* short */
            jobject val = env->newObject(env->findClass("java/lang/Short"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt32((int16_t)ret);
            return val;
        }
        case 'I': { /* integer */
            jobject val = env->newObject(env->findClass("java/lang/Integer"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt32((int32_t)ret);
            return val;
        }
        case 'F': { /* float */
            jobject val = env->newObject(env->findClass("java/lang/Float"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt32((int32_t)ret);
            return val;
        }
        case 'D': { /* double */
            jobject val = env->newObject(env->findClass("java/lang/Double"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt64(ret);
            return val;
        }
        case 'J': { /* long */
            jobject val = env->newObject(env->findClass("java/lang/Long"));
            if(val == NULL) return NULL;
            val->getFieldByIndex(0)->setInt64(ret);
            return val;
        }
        default:
            return (jobject)ret;
    }
}
