
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_native_system_class.h"

static bool nativeCurrentTimeMillis(Execution &execution) {
    // TODO
    return 0;
}

static bool nativeNanoTime(Execution &execution) {
    // TODO
    return 0;
}

static bool nativeArraycopy(Execution &execution) {
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
                    ((uint8_t *)dstVal)[i] = ((uint8_t *)srcVal)[i];
                break;
            case 2:
                for(uint32_t i = 0; i < length; i++)
                    ((uint16_t *)dstVal)[i] = ((uint16_t *)srcVal)[i];
                break;
            case 4:
                for(uint32_t i = 0; i < length; i++)
                    ((uint32_t *)dstVal)[i] = ((uint32_t *)srcVal)[i];
                break;
            case 8:
                for(uint32_t i = 0; i < length; i++)
                    ((uint64_t *)dstVal)[i] = ((uint64_t *)srcVal)[i];
                break;
        }
    }
    else {
        throw "ArrayStoreException";
        return false;
    }
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00""currentTimeMillis", "\x03\x00""()J",                                        nativeCurrentTimeMillis),
    NATIVE_METHOD("\x08\x00""nanoTime",          "\x03\x00""()J",                                        nativeNanoTime),
    NATIVE_METHOD("\x09\x00""arraycopy",         "\x2A\x00""(Ljava/lang/Object;ILjava/lang/Object;II)V", nativeArraycopy),
};

const NativeClass SYSTEM_CLASS = NATIVE_CLASS(systemClass, methods);
