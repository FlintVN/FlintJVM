
#include "mjvm.h"
#include "mjvm_object.h"
#include "mjvm_system_api.h"
#include "mjvm_const_name.h"
#include "mjvm_native_system_class.h"

static bool nativeCurrentTimeMillis(MjvmExecution &execution) {
    execution.stackPushInt64(MjvmSystem_GetNanoTime() / 1000);
    return true;
}

static bool nativeNanoTime(MjvmExecution &execution) {
    execution.stackPushInt64(MjvmSystem_GetNanoTime());
    return true;
}

static bool nativeArraycopy(MjvmExecution &execution) {
    int32_t length = execution.stackPopInt32();
    int32_t destPos = execution.stackPopInt32();
    MjvmObject *dest = execution.stackPopObject();
    int32_t srcPos = execution.stackPopInt32();
    MjvmObject *src = execution.stackPopObject();
    if(src->type == dest->type) {
        uint8_t atype = MjvmObject::isPrimType(src->type);
        uint8_t elementSize = atype ? MjvmObject::getPrimitiveTypeSize(atype) : sizeof(MjvmObject *);
        if((length < 0) || ((length + srcPos) > src->size / elementSize) || ((length + destPos) > dest->size / elementSize))
            throw "Index out of range in System.arraycopy";
        void *srcVal = src->data;
        void *dstVal = dest->data;
        switch(elementSize) {
            case 1:
                for(uint32_t i = 0; i < length; i++)
                    ((uint8_t *)dstVal)[i + destPos] = ((uint8_t *)srcVal)[i + srcPos];
                break;
            case 2:
                for(uint32_t i = 0; i < length; i++)
                    ((uint16_t *)dstVal)[i + destPos] = ((uint16_t *)srcVal)[i + srcPos];
                break;
            case 4:
                for(uint32_t i = 0; i < length; i++)
                    ((uint32_t *)dstVal)[i + destPos] = ((uint32_t *)srcVal)[i + srcPos];
                break;
            case 8:
                for(uint32_t i = 0; i < length; i++)
                    ((uint64_t *)dstVal)[i + destPos] = ((uint64_t *)srcVal)[i + srcPos];
                break;
        }
    }
    else {
        MjvmString *strObj = execution.mjvm.newString(STR_AND_SIZE("Type mismatch, can not copy array object"));
        MjvmThrowable *excpObj = execution.mjvm.newArrayStoreException(strObj);
        execution.stackPushObject(excpObj);
        return false;
    }
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\xFC\x06""currentTimeMillis", "\x03\x00\x9B\x00""()J",                                        nativeCurrentTimeMillis),
    NATIVE_METHOD("\x08\x00\x3B\x03""nanoTime",          "\x03\x00\x9B\x00""()J",                                        nativeNanoTime),
    NATIVE_METHOD("\x09\x00\xDA\x03""arraycopy",         "\x2A\x00\x82\x0E""(Ljava/lang/Object;ILjava/lang/Object;II)V", nativeArraycopy),
};

const NativeClass SYSTEM_CLASS = NATIVE_CLASS(systemClassName, methods);
