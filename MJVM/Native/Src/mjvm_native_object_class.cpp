
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_system_class.h"

static int64_t nativeGetClass(int32_t args[], int32_t argc) {
    // TODO
    return 0;
}

static int64_t nativeHashCode(int32_t args[], int32_t argc) {
    MjvmObject *obj = (MjvmObject *)args[0];
    return (int64_t)obj;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00""getClass", "\x09\x00""()LClass;", nativeGetClass),
    NATIVE_METHOD("\x08\x00""hashCode", "\x03\x00""()I",       nativeHashCode),
};

const NativeClass OBJECT_CLASS = NATIVE_CLASS(objectClass, methods);
