
#include <string.h>
#include "flint.h"
#include "flint_object.h"
#include "flint_const_name.h"
#include "flint_native_object_class.h"

static bool nativeGetClass(FlintExecution &execution) {
    uint32_t idx = 0;
    FlintObject *obj = execution.stackPopObject();
    uint16_t length = obj->type.length;
    bool isPrim = FlintObject::isPrimType(obj->type);
    if(obj->dimensions) {
        length += obj->dimensions;
        if(!isPrim)
            length += 2;
    }
    FlintString &strObj = execution.flint.newString(length, 0);
    FlintObject *byteArray = strObj.getValue();
    if(obj->dimensions) {
        for(uint32_t i = 0; i < obj->dimensions; i++)
            byteArray->data[idx++] = '[';
        if(!isPrim)
            byteArray->data[idx++] = 'L';
    }
    for(uint32_t i = 0; i < obj->type.length; i++) {
        uint8_t c = obj->type.text[i];
        byteArray->data[idx++] = (c == '/') ? '.' : c;
    }
    if(obj->dimensions && !isPrim)
        byteArray->data[idx++] = ';';
    execution.stackPushObject(&execution.flint.getConstClass(strObj));
    return true;
}

static bool nativeHashCode(FlintExecution &execution) {
    FlintObject *obj = execution.stackPopObject();
    execution.stackPushInt32((int32_t)obj);
    return true;
}

static bool nativeClone(FlintExecution &execution) {
    FlintObject *obj = execution.stackPopObject();
    if(obj->dimensions > 0) {
        FlintObject &cloneObj = execution.flint.newObject(obj->size, obj->type, obj->dimensions);
        memcpy(cloneObj.data, obj->data, obj->size);
        execution.stackPushObject(&cloneObj);
        return true;
    }
    else {
        FlintString &strObj = execution.flint.newString(STR_AND_SIZE("Clone method is not supported"));
        FlintThrowable &excpObj = execution.flint.newCloneNotSupportedException(strObj);
        execution.stackPushObject(&excpObj);
        return false;
    }
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00\xAA\x1C""getClass", "\x13\x00\x0A\x1F""()Ljava/lang/Class;",  nativeGetClass),
    NATIVE_METHOD("\x08\x00\x6D\x04""hashCode", "\x03\x00\xD0\x51""()I",                  nativeHashCode),
    NATIVE_METHOD("\x05\x00\xDE\xF1""clone",    "\x14\x00\xC7\x39""()Ljava/lang/Object;", nativeClone),
};

const FlintNativeClass OBJECT_CLASS = NATIVE_CLASS(objectClassName, methods);
