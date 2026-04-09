
#include "flint_array_object.h"
#include "flint_native_common.h"

bool CheckIndex(FNIEnv *env, jarray array, int32_t index) {
    if(index < 0 || index >= array->getLength()) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = array->getCompTypeName(&len);
        env->throwNew(excpCls, "index %d out of bounds for %.*s[%d]", index, len, name, array->getLength());
        return false;
    }
    return true;
}

bool CheckArrayIndexSize(FNIEnv *env, jarray arr, int32_t index, int32_t count) {
    if(arr == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    else if(index < 0) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = arr->getCompTypeName(&len);
        env->throwNew(excpCls, "index %d out of bounds for %.*s[%d]", index, len, name, arr->getLength());
        return false;
    }
    else if((index + count) > arr->getLength()) {
        jclass excpCls = env->findClass("java/lang/ArrayIndexOutOfBoundsException");
        uint16_t len;
        const char *name = arr->getCompTypeName(&len);
        env->throwNew(excpCls, "last index %d out of bounds for %.*s[%d]", index + count - 1, len, name, arr->getLength());
        return false;
    }
    return true;
}
