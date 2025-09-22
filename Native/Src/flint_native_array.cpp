
#include <string.h>
#include "flint.h"
#include "flint_common.h"
#include "flint_java_class.h"
#include "flint_array_object.h"
#include "flint_native_array.h"

static bool checkIsArray(FNIEnv *env, jobject obj) {
    if(obj == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    if(!obj->isArray()) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "object type %s is not an array", obj->getTypeName());
        return false;
    }
    return true;
}

static bool checkIsClassType(FNIEnv *env, jobject obj) {
    if(obj == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    else if(obj->type != NULL) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "componentType %s is not a Class object", obj->getTypeName());
        return false;
    }
    return true;
}

static bool checkIndex(FNIEnv *env, jarray array, int32_t index) {
    if(index < 0 || index >= array->getLength()) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = array->getCompTypeName(&len);
        env->throwNew(excpCls, "index %d out of bounds for %.*s[%d]", index, len, name, array->getLength());
        return false;
    }
    return true;
}

static bool checkDimensions(FNIEnv *env, jintArray dimensions) {
    if(dimensions == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    if(dimensions->isArray() == false || dimensions->getTypeName()[1] != 'I') {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "dimensions %s is not an array of int[]", dimensions->getTypeName());
        return false;
    }
    uint32_t depth = dimensions->getLength();
    if(depth == 0) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "dimensions array is empty");
        return false;
    }
    else if(depth > 255) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "dimensions array cannot be larger than 255 elements");
        return false;
    }
    jint *data = dimensions->getData();
    for(uint32_t i = 0; i < depth; i++) {
        if(data[i] < 0) {
            env->throwNew(env->findClass("java/lang/NegativeArraySizeException"));
            return false;
        }
    }
    return true;
}

static char isArrayOfPrimative(jobject obj) {
    const char *typeName = obj->getTypeName();
    if(typeName[2] == 0) switch(typeName[1]) {
        case 'Z':
        case 'C':
        case 'F':
        case 'D':
        case 'B':
        case 'S':
        case 'I':
        case 'J':
        case 'V': return typeName[1];
        default: return 0;
    }
    return 0;
}

jint nativeArrayGetLength(FNIEnv *env, jobject obj) {
    (void)env;
    if(checkIsArray(env, obj) == false) return 0;
    return ((jarray)obj)->getLength();
}

jobject nativeArrayGet(FNIEnv *env, jobject obj, jint index) {
    if(checkIsArray(env, obj) == false) return NULL;
    if(checkIndex(env, (jarray)obj, index) == false) return NULL;
    char c = isArrayOfPrimative(obj);
    if(c != 0) {
        switch(c) {
            case 'B': { /* byte */
                jobject val = env->newObject(env->findClass("java/lang/Byte"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jbyteArray)obj)->getData()[index];
                return val;
            }
            case 'Z': { /* boolean */
                jobject val = env->newObject(env->findClass("java/lang/Boolean"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jboolArray)obj)->getData()[index];
                return val;
            }
            case 'C': { /* char */
                jobject val = env->newObject(env->findClass("java/lang/Character"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jcharArray)obj)->getData()[index];
                return val;
            }
            case 'S': { /* short */
                jobject val = env->newObject(env->findClass("java/lang/Short"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jshortArray)obj)->getData()[index];
                return val;
            }
            case 'I': { /* integer */
                jobject val = env->newObject(env->findClass("java/lang/Integer"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jintArray)obj)->getData()[index];
                return val;
            }
            case 'F': { /* float */
                jobject val = env->newObject(env->findClass("java/lang/Float"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jfloatArray)obj)->getData()[index];
                return val;
            }
            case 'D': { /* double */
                jobject val = env->newObject(env->findClass("java/lang/Double"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jdoubleArray)obj)->getData()[index];
                return val;
            }
            default: { /* long */
                jobject val = env->newObject(env->findClass("java/lang/Long"));
                if(val == NULL) return NULL;
                val->getField32ByIndex(0)->value = ((jlongArray)obj)->getData()[index];
                return val;
            }
        }
    }
    return ((jobjectArray)obj)->getData()[index];
}

jbool nativeArrayGetBoolean(FNIEnv *env, jobject obj, jint index) {
    if(isArrayOfPrimative(obj) != 'Z') {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "object type %s is not a boolean[] array", obj->getTypeName());
        return false;
    }
    if(checkIndex(env, (jarray)obj, index) == false) return false;
    return ((jboolArray)obj)->getData()[index];
}

jbyte nativeArrayGetByte(FNIEnv *env, jobject obj, jint index) {
    if(isArrayOfPrimative(obj) != 'B') {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "object type %s is not a byte[] array", obj->getTypeName());
        return false;
    }
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    return ((jbyteArray)obj)->getData()[index];
}

jchar nativeArrayGetChar(FNIEnv *env, jobject obj, jint index) {
    if(isArrayOfPrimative(obj) != 'C') {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "object type %s is not a char[] array", obj->getTypeName());
        return false;
    }
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    return ((jcharArray)obj)->getData()[index];
}

jshort nativeArrayGetShort(FNIEnv *env, jobject obj, jint index) {
    if(checkIsArray(env, obj) == false) return 0;
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    switch(isArrayOfPrimative(obj)) {
        case 'B': /* byte */
            return ((jbyteArray)obj)->getData()[index];
        case 'S': /* short */
            return ((jshortArray)obj)->getData()[index];
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            env->throwNew(excpCls, "cannot get short value from object type %s", obj->getTypeName());
            return 0;
        }
    }
}

jint nativeArrayGetInt(FNIEnv *env, jobject obj, jint index) {
    if(checkIsArray(env, obj) == false) return 0;
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    switch(isArrayOfPrimative(obj)) {
        case 'B': /* byte */
            return ((jbyteArray)obj)->getData()[index];
        case 'C': /* char */
        case 'S': /* short */
            return ((jshortArray)obj)->getData()[index];
        case 'I': /* integer */
            return ((jintArray)obj)->getData()[index];
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            env->throwNew(excpCls, "cannot get int value from object type %s", obj->getTypeName());
            return 0;
        }
    }
}

jlong nativeArrayGetLong(FNIEnv *env, jobject obj, jint index) {
    if(checkIsArray(env, obj) == false) return 0;
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    switch(isArrayOfPrimative(obj)) {
        case 'B': /* byte */
            return ((jbyteArray)obj)->getData()[index];
        case 'C': /* char */
        case 'S': /* short */
            return ((jshortArray)obj)->getData()[index];
        case 'I': /* integer */
            return ((jintArray)obj)->getData()[index];
        case 'J': /* long */
            return ((jlongArray)obj)->getData()[index];
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            env->throwNew(excpCls, "cannot get long value from object type %s", obj->getTypeName());
            return 0;
        }
    }
}

jfloat nativeArrayGetFloat(FNIEnv *env, jobject obj, jint index) {
    if(checkIsArray(env, obj) == false) return 0;
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    switch(isArrayOfPrimative(obj)) {
        case 'B': /* byte */
            return ((jbyteArray)obj)->getData()[index];
        case 'C': /* char */
        case 'S': /* short */
            return ((jshortArray)obj)->getData()[index];
        case 'I': /* integer */
            return ((jintArray)obj)->getData()[index];
        case 'F': /* float */
            return ((jfloatArray)obj)->getData()[index];
        case 'J': /* long */
            return ((jlongArray)obj)->getData()[index];
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            env->throwNew(excpCls, "cannot get float value from object type %s", obj->getTypeName());
            return 0;
        }
    }
}

jdouble nativeArrayGetDouble(FNIEnv *env, jobject obj, jint index) {
    if(checkIsArray(env, obj) == false) return 0;
    if(checkIndex(env, (jarray)obj, index) == false) return 0;
    switch(isArrayOfPrimative(obj)) {
        case 'B': /* byte */
            return ((jbyteArray)obj)->getData()[index];
        case 'C': /* char */
        case 'S': /* short */
            return ((jshortArray)obj)->getData()[index];
        case 'I': /* integer */
            return ((jintArray)obj)->getData()[index];
        case 'F': /* float */
            return ((jfloatArray)obj)->getData()[index];
        case 'J': /* long */
            return ((jlongArray)obj)->getData()[index];
        case 'D': /* double */
            return ((jdoubleArray)obj)->getData()[index];
        default:  {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            env->throwNew(excpCls, "cannot get double value from object type %s", obj->getTypeName());
            return 0;
        }
    }
}

static char parseWrapperClass(jobject obj) {
    const char *typeName = obj->getTypeName();
    uint32_t hash = Hash(typeName);
    switch(hash) {
        case Hash("java/lang/Byte"):
            return (strcmp(typeName, "java/lang/Byte") == 0) ? 'B' : 0;
        case Hash("java/lang/Short"):
            return (strcmp(typeName, "java/lang/Short") == 0) ? 'S' : 0; 
        case Hash("java/lang/Integer"):
            return (strcmp(typeName, "java/lang/Integer") == 0) ? 'I' : 0; 
        case Hash("java/lang/Long"):
            return (strcmp(typeName, "java/lang/Long") == 0) ? 'J' : 0; 
        case Hash("java/lang/Float"):
            return (strcmp(typeName, "java/lang/Float") == 0) ? 'F' : 0; 
        case Hash("java/lang/Double"):
            return (strcmp(typeName, "java/lang/Double") == 0) ? 'D' : 0; 
        case Hash("java/lang/Character"):
            return (strcmp(typeName, "java/lang/Character") == 0) ? 'C' : 0; 
        case Hash("java/lang/Boolean"):
            return (strcmp(typeName, "java/lang/Boolean") == 0) ? 'Z' : 0; 
        default:
            return 0;
    }
}

jvoid nativeArraySet(FNIEnv *env, jobject obj, jint index, jobject v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    char c = isArrayOfPrimative(obj);
    if(c != 0) {
        if(v == NULL) return env->throwNew(env->findClass("java/lang/NullPointerException"));
        char vc = parseWrapperClass(obj);
        switch(c) {
            case 'Z': { /* boolean */
                if(vc == 'Z') ((jboolArray)obj)->getData()[index] = (jbool)v->getField32ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            case 'B': { /* byte */
                if(vc == 'B') ((jbyteArray)obj)->getData()[index] = (jbyte)v->getField32ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            case 'C': { /* char */
                if(vc == 'C') ((jcharArray)obj)->getData()[index] = (jchar)v->getField32ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            case 'S': { /* short */
                if(vc == 'B') ((jshortArray)obj)->getData()[index] = (jshort)v->getField32ByIndex(0)->value;
                else if(vc == 'S') ((jshortArray)obj)->getData()[index] = (jshort)v->getField32ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            case 'I': { /* integer */
                if(vc == 'B') ((jintArray)obj)->getData()[index] = (jint)v->getField32ByIndex(0)->value;
                else if(vc == 'C') ((jintArray)obj)->getData()[index] = (jint)v->getField32ByIndex(0)->value;
                else if(vc == 'S') ((jintArray)obj)->getData()[index] = (jint)v->getField32ByIndex(0)->value;
                else if(vc == 'I') ((jintArray)obj)->getData()[index] = (jint)v->getField32ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            case 'F': { /* float */
                if(vc == 'B') ((jfloatArray)obj)->getData()[index] = (jfloat)v->getField32ByIndex(0)->value;
                else if(vc == 'C') ((jfloatArray)obj)->getData()[index] = (jfloat)v->getField32ByIndex(0)->value;
                else if(vc == 'S') ((jfloatArray)obj)->getData()[index] = (jfloat)v->getField32ByIndex(0)->value;
                else if(vc == 'I') ((jfloatArray)obj)->getData()[index] = (jfloat)v->getField32ByIndex(0)->value;
                else if(vc == 'F') ((jfloatArray)obj)->getData()[index] = *(jfloat *)&v->getField32ByIndex(0)->value;
                else if(vc == 'J') ((jfloatArray)obj)->getData()[index] = (jfloat)*(jdouble *)&v->getField64ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            case 'D': { /* double */
                if(vc == 'B') ((jdoubleArray)obj)->getData()[index] = (jdouble)v->getField32ByIndex(0)->value;
                else if(vc == 'C') ((jdoubleArray)obj)->getData()[index] = (jdouble)v->getField32ByIndex(0)->value;
                else if(vc == 'S') ((jdoubleArray)obj)->getData()[index] = (jdouble)v->getField32ByIndex(0)->value;
                else if(vc == 'I') ((jdoubleArray)obj)->getData()[index] = (jdouble)v->getField32ByIndex(0)->value;
                else if(vc == 'F') ((jdoubleArray)obj)->getData()[index] = (jdouble)*(jfloat *)&v->getField32ByIndex(0)->value;
                else if(vc == 'J') ((jdoubleArray)obj)->getData()[index] = (jdouble)v->getField64ByIndex(0)->value;
                else if(vc == 'D') ((jdoubleArray)obj)->getData()[index] = *(jdouble *)&v->getField64ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
            default: { /* long */
                if(vc == 'B') ((jlongArray)obj)->getData()[index] = (jlong)v->getField32ByIndex(0)->value;
                else if(vc == 'C') ((jlongArray)obj)->getData()[index] = (jlong)v->getField32ByIndex(0)->value;
                else if(vc == 'S') ((jlongArray)obj)->getData()[index] = (jlong)v->getField32ByIndex(0)->value;
                else if(vc == 'I') ((jlongArray)obj)->getData()[index] = (jlong)v->getField32ByIndex(0)->value;
                else if(vc == 'J') ((jlongArray)obj)->getData()[index] = (jlong)v->getField64ByIndex(0)->value;
                else return env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "argument type mismatch");
                return;
            }
        }
    }
    else ((jobjectArray)obj)->getData()[index] = v;
}

jvoid nativeArraySetBoolean(FNIEnv *env, jobject obj, jint index, jbool v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    if(isArrayOfPrimative(obj) != 'Z') {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        return env->throwNew(excpCls, "object type %s is not a boolean[] array", obj->getTypeName());
    }
    ((jbyteArray)obj)->getData()[index] = !!v;
}

jvoid nativeArraySetByte(FNIEnv *env, jobject obj, jint index, jbyte v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    switch(isArrayOfPrimative(obj)) {
        case 'B': /* byte */
            ((jbyteArray)obj)->getData()[index] = v;
            return;
        case 'S': /* short */
            ((jshortArray)obj)->getData()[index] = v;
            return;
        case 'I': /* integer */
            ((jintArray)obj)->getData()[index] = v;
            return;
        case 'F': /* float */
            ((jfloatArray)obj)->getData()[index] = v;
            return;
        case 'J': /* long */
            ((jlongArray)obj)->getData()[index] = v;
            return;
        case 'D': /* double */
            ((jdoubleArray)obj)->getData()[index] = v;
            return;
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            return env->throwNew(excpCls, "cannot set byte value from object type %s", obj->getTypeName());
        }
    }
}

jvoid nativeArraySetChar(FNIEnv *env, jobject obj, jint index, jchar v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    switch(isArrayOfPrimative(obj)) {
        case 'C': /* char */
            ((jshortArray)obj)->getData()[index] = v;
            return;
        case 'I': /* integer */
            ((jintArray)obj)->getData()[index] = v;
            return;
        case 'F': /* float */
            ((jfloatArray)obj)->getData()[index] = v;
            return;
        case 'J': /* long */
            ((jlongArray)obj)->getData()[index] = v;
            return;
        case 'D': /* double */
            ((jdoubleArray)obj)->getData()[index] = v;
            return;
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            return env->throwNew(excpCls, "cannot set char value from object type %s", obj->getTypeName());
        }
    }
}

jvoid nativeArraySetShort(FNIEnv *env, jobject obj, jint index, jshort v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    switch(isArrayOfPrimative(obj)) {
        case 'S': /* short */
            ((jshortArray)obj)->getData()[index] = v;
            return;
        case 'I': /* integer */
            ((jintArray)obj)->getData()[index] = v;
            return;
        case 'F': /* float */
            ((jfloatArray)obj)->getData()[index] = v;
            return;
        case 'J': /* long */
            ((jlongArray)obj)->getData()[index] = v;
            return;
        case 'D': /* double */
            ((jdoubleArray)obj)->getData()[index] = v;
            return;
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            return env->throwNew(excpCls, "cannot set char value from object type %s", obj->getTypeName());
        }
    }
}

jvoid nativeArraySetInt(FNIEnv *env, jobject obj, jint index, jint v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    switch(isArrayOfPrimative(obj)) {
        case 'I': /* integer */
            ((jintArray)obj)->getData()[index] = v;
            return;
        case 'F': /* float */
            ((jfloatArray)obj)->getData()[index] = v;
            return;
        case 'J': /* long */
            ((jlongArray)obj)->getData()[index] = v;
            return;
        case 'D': /* double */
            ((jdoubleArray)obj)->getData()[index] = v;
            return;
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            return env->throwNew(excpCls, "cannot set int value from object type %s", obj->getTypeName());
        }
    }
}

jvoid nativeArraySetLong(FNIEnv *env, jobject obj, jint index, jlong v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    switch(isArrayOfPrimative(obj)) {
        case 'F': /* float */
            ((jfloatArray)obj)->getData()[index] = v;
            return;
        case 'J': /* long */
            ((jlongArray)obj)->getData()[index] = v;
            return;
        case 'D': /* double */
            ((jdoubleArray)obj)->getData()[index] = v;
            return;
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            return env->throwNew(excpCls, "cannot set long value from object type %s", obj->getTypeName());
        }
    }
}

jvoid nativeArraySetFloat(FNIEnv *env, jobject obj, jint index, jfloat v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    switch(isArrayOfPrimative(obj)) {
        case 'F': /* float */
            ((jfloatArray)obj)->getData()[index] = v;
            return;
        case 'D': /* double */
            ((jdoubleArray)obj)->getData()[index] = v;
            return;
        default: {
            jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
            return env->throwNew(excpCls, "cannot set float value from object type %s", obj->getTypeName());
        }
    }
}

jvoid nativeArraySetDouble(FNIEnv *env, jobject obj, jint index, jdouble v) {
    if(checkIsArray(env, obj) == false) return;
    if(checkIndex(env, (jarray)obj, index) == false) return;
    if(isArrayOfPrimative(obj) != 'D') {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        return env->throwNew(excpCls, "object type %s is not a double[] array", obj->getTypeName());
    }
    ((jdoubleArray)obj)->getData()[index] = v;
}

jobject nativeArrayNewArray(FNIEnv *env, jclass componentType, jint length) {
    if(checkIsClassType(env, componentType) == false) return NULL;
    if(length < 0) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        env->throwNew(excpCls, "length -%d is negative", length);
        return NULL;
    }
    if(componentType->isPrimitive()) {
        switch(JClass::isPrimitive(componentType->getTypeName())) {
            case 'Z': return env->newBoolArray(length);
            case 'C': return env->newCharArray(length);
            case 'F': return env->newFloatArray(length);
            case 'D': return env->newDoubleArray(length);
            case 'B': return env->newByteArray(length);
            case 'S': return env->newShortArray(length);
            case 'I': return env->newIntArray(length);
            default: return env->newLongArray(length);
        }
    }
    else {
        jclass cls = Flint::findClassOfArray(env->exec, componentType->getTypeName(), 1);
        return Flint::newArray(env->exec, cls, length);
    }
}

jobject nativeArrayMultiNewArray(FNIEnv *env, jclass componentType, jintArray dimensions) {
    if(checkIsClassType(env, componentType) == false) return NULL;
    if(checkDimensions(env, dimensions) == false) return NULL;
    if(componentType->isPrimitive()) {
        char type[2];
        type[0] = JClass::isPrimitive(componentType->getTypeName());
        type[1] = 0;
        jclass cls = Flint::findClassOfArray(env->exec, type, dimensions->getLength());
        return Flint::newMultiArray(env->exec, cls, dimensions->getData(), dimensions->getLength());
    }
    jclass cls = Flint::findClassOfArray(env->exec, componentType->getTypeName(), dimensions->getLength());
    return Flint::newMultiArray(env->exec, cls, dimensions->getData(), dimensions->getLength());
}
