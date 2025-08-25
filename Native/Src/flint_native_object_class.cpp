
#include <string.h>
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_object_class.h"
#include "flint_throw_support.h"

static FlintError nativeGetClass(FlintExecution *exec) {
    uint32_t idx = 0;
    FlintJavaObject *obj = exec->stackPopObject();
    uint16_t length = obj->type.length;
    bool isPrim = FlintJavaObject::isPrimType(obj->type);
    if(obj->dimensions) {
        length += obj->dimensions;
        if(!isPrim)
            length += 2;
    }
    auto strObj = exec->flint.newString(length, 0);
    if(strObj.err != ERR_OK)
        return checkAndThrowForFlintError(exec, strObj.err, strObj.getErrorMsg(), strObj.getErrorMsgLength());
    int8_t *byteArray = strObj.value->getValue()->getData();
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
    auto cls = exec->flint.getConstClass(*strObj.value);
    if(cls.err != ERR_OK) {
        exec->flint.freeObject(strObj.value->getValue());
        exec->flint.freeObject(strObj.value);
        return checkAndThrowForFlintError(exec, cls.err, cls.getErrorMsg(), cls.getErrorMsgLength());
    }
    exec->stackPushObject(cls.value);
    return ERR_OK;
}

static FlintError nativeHashCode(FlintExecution *exec) {
    FlintJavaObject *obj = exec->stackPopObject();
    exec->stackPushInt32((int32_t)obj);
    return ERR_OK;
}

static FlintError nativeClone(FlintExecution *exec) {
    FlintJavaObject *obj = exec->stackPopObject();
    if(obj->dimensions > 0) {
        auto cloneObj = exec->flint.newObject(obj->size, &obj->type, obj->dimensions);
        if(cloneObj.err != ERR_OK)
            return checkAndThrowForFlintError(exec, cloneObj.err, cloneObj.getErrorMsg(), cloneObj.getErrorMsgLength());
        memcpy(((FlintInt8Array *)cloneObj.value)->getData(), ((FlintInt8Array *)obj)->getData(), ((FlintInt8Array *)obj)->getLength());
        exec->stackPushObject(cloneObj.value);
    }
    else
        return throwCloneNotSupportedException(exec, "Clone method is not supported");
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x08\x00\xAA\x1C""getClass", "\x13\x00\x0A\x1F""()Ljava/lang/Class;",  nativeGetClass),
    NATIVE_METHOD("\x08\x00\x6D\x04""hashCode", "\x03\x00\xD0\x51""()I",                  nativeHashCode),
    NATIVE_METHOD("\x05\x00\xDE\xF1""clone",    "\x14\x00\xC7\x39""()Ljava/lang/Object;", nativeClone),
};

const FlintNativeClass OBJECT_CLASS = NATIVE_CLASS(objectClassName, methods);
