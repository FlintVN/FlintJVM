
#include "mjvm_object.h"
#include "mjvm_const_name.h"
#include "mjvm_system_class.h"

int64_t nativeCurrentTimeMillis(int32_t args[], int32_t argc) {
    // TODO
    return 0;
}

int64_t nativeNanoTime(int32_t args[], int32_t argc) {
    // TODO
    return 0;
}

int64_t nativeArraycopy(int32_t args[], int32_t argc) {
    MjvmObject *src = (MjvmObject *)args[0];
    int32_t srcPos = args[1];
    MjvmObject *dest = (MjvmObject *)args[2];
    int32_t destPos = args[3];
    int32_t length = args[4];
    if(src->type == dest->type) {
        uint8_t elementSize = MjvmObject::isPrimType(src->type);
        elementSize = (elementSize == 0) ? sizeof(MjvmObject *) : MjvmObject::getPrimitiveTypeSize(elementSize);
        if((length < 0) || ((length + srcPos) > src->size / elementSize) || ((length + destPos) > dest->size / elementSize))
            throw "Index out of range in System.arraycopy";
        void *srcVal = src->data;
        void *dstVal = dest->data;
        switch(elementSize) {
            case 4:  // boolean
            case 8:  // byte
                for(uint32_t i = 0; i < length; i++)
                    ((uint8_t *)dstVal)[i] = ((uint8_t *)srcVal)[i];
                break;
            case 5:  // char
            case 9:  // short
                for(uint32_t i = 0; i < length; i++)
                    ((uint16_t *)dstVal)[i] = ((uint16_t *)srcVal)[i];
                break;
            case 7:  // double
            case 11: // long
                for(uint32_t i = 0; i < length; i++)
                    ((uint64_t *)dstVal)[i] = ((uint64_t *)srcVal)[i];
                break;
            default: // object
                for(uint32_t i = 0; i < length; i++)
                    ((uint32_t *)dstVal)[i] = ((uint32_t *)srcVal)[i];
                break;
        }
    }
    else
        throw "Type mismatch";
    return 0;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00""currentTimeMillis", "\x03\x00""()J",                                        nativeCurrentTimeMillis),
    NATIVE_METHOD("\x08\x00""nanoTime",          "\x03\x00""()J",                                        nativeNanoTime),
    NATIVE_METHOD("\x09\x00""arraycopy",         "\x2A\x00""(Ljava/lang/Object;ILjava/lang/Object;II)V", nativeArraycopy),
};

const NativeClass SYSTEM_CLASS = NATIVE_CLASS(systemClass, methods);
