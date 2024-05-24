
#include <string.h>
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_object_class.h"

static bool nativeGetClass(Execution &execution) {
    // TODO
    return true;
}

static bool nativeHashCode(Execution &execution) {
    MjvmObject *obj = execution.stackPopObject();
    execution.stackPushInt32((int32_t)obj);
    return true;
}

static bool nativeClone(Execution &execution) {
    MjvmObject *obj = execution.stackPopObject();
    if(obj->dimensions > 0) {
        MjvmObject *cloneObj = execution.newObject(obj->size, obj->type, obj->dimensions);
        memcpy(cloneObj->data, obj->data, obj->size);
        execution.stackPushObject(cloneObj);
        return true;
    }
    else {
        MjvmString *strObj = execution.newString(STR_AND_SIZE("Clone method is not supported"));
        MjvmThrowable *excpObj = execution.newCloneNotSupportedException(strObj);
        execution.stackPushObject(excpObj);
        return false;
    }
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00""getClass", "\x09\x00""()LClass;",            nativeGetClass),
    NATIVE_METHOD("\x08\x00""hashCode", "\x03\x00""()I",                  nativeHashCode),
    NATIVE_METHOD("\x05\x00""clone",    "\x14\x00""()Ljava/lang/Object;", nativeClone),
};

const NativeClass OBJECT_CLASS = NATIVE_CLASS(objectClassName, methods);
