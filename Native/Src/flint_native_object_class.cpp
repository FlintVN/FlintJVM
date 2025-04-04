
#include <string.h>
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_object_class.h"

static void nativeGetClass(FlintExecution &execution) {
    uint32_t idx = 0;
    FlintJavaObject *obj = execution.stackPopObject();
    uint16_t length = obj->type.length;
    bool isPrim = FlintJavaObject::isPrimType(obj->type);
    if(obj->dimensions) {
        length += obj->dimensions;
        if(!isPrim)
            length += 2;
    }
    FlintJavaString &strObj = execution.flint.newString(length, 0);
    int8_t *byteArray = strObj.getValue()->getData();
    if(obj->dimensions) {
        for(uint32_t i = 0; i < obj->dimensions; i++)
            byteArray[idx++] = '[';
        if(!isPrim)
            byteArray[idx++] = 'L';
    }
    for(uint32_t i = 0; i < obj->type.length; i++) {
        uint8_t c = obj->type.text[i];
        byteArray[idx++] = (c == '/') ? '.' : c;
    }
    if(obj->dimensions && !isPrim)
        byteArray[idx++] = ';';
    execution.stackPushObject(&execution.flint.getConstClass(strObj));
}

static void nativeHashCode(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    execution.stackPushInt32((int32_t)obj);
}

static void nativeClone(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    if(obj->dimensions > 0) {
        FlintJavaObject &cloneObj = execution.flint.newObject(obj->size, obj->type, obj->dimensions);
        memcpy(((FlintInt8Array &)cloneObj).getData(), ((FlintInt8Array *)obj)->getData(), ((FlintInt8Array *)obj)->getLength());
        execution.stackPushObject(&cloneObj);
    }
    else {
        FlintJavaString &strObj = execution.flint.newString(STR_AND_SIZE("Clone method is not supported"));
        throw &execution.flint.newCloneNotSupportedException(&strObj);
    }
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00\xAA\x1C""getClass", "\x13\x00\x0A\x1F""()Ljava/lang/Class;",  nativeGetClass),
    NATIVE_METHOD("\x08\x00\x6D\x04""hashCode", "\x03\x00\xD0\x51""()I",                  nativeHashCode),
    NATIVE_METHOD("\x05\x00\xDE\xF1""clone",    "\x14\x00\xC7\x39""()Ljava/lang/Object;", nativeClone),
};

const FlintNativeClass OBJECT_CLASS = NATIVE_CLASS(objectClassName, methods);
