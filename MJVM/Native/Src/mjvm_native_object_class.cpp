
#include <string.h>
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_object_class.h"

static bool nativeGetClass(MjvmExecution &execution) {
    uint32_t idx = 0;
    MjvmObject *obj = execution.stackPopObject();
    uint16_t length = obj->type.length;
    bool isPrim = MjvmObject::isPrimType(obj->type);
    if(obj->dimensions) {
        length += obj->dimensions;
        if(!isPrim)
            length += 2;
    }
    MjvmString *strObj = execution.newString(length, 0);
    MjvmObject *byteArray = ((MjvmFieldsData *)strObj->data)->getFieldObject(*(MjvmConstNameAndType *)stringValueFieldName).object;
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
    execution.stackPushObject(execution.getConstClass(*strObj));
    return true;
}

static bool nativeHashCode(MjvmExecution &execution) {
    MjvmObject *obj = execution.stackPopObject();
    execution.stackPushInt32((int32_t)obj);
    return true;
}

static bool nativeClone(MjvmExecution &execution) {
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
    NATIVE_METHOD("\x08\x00\x36\x03""getClass", "\x13\x00\x70\x06""()Ljava/lang/Class;",  nativeGetClass),
    NATIVE_METHOD("\x08\x00\x1F\x03""hashCode", "\x03\x00\x9A\x00""()I",                  nativeHashCode),
    NATIVE_METHOD("\x05\x00\x11\x02""clone",    "\x14\x00\xD1\x06""()Ljava/lang/Object;", nativeClone),
};

const NativeClass OBJECT_CLASS = NATIVE_CLASS(objectClassName, methods);
