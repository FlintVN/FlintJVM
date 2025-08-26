
#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "flint_const_name_base.h"
#include "flint_native_system_class.h"
#include "flint_throw_support.h"

static FlintError nativeCurrentTimeMillis(FlintExecution *exec) {
    exec->stackPushInt64(FlintAPI::System::getNanoTime() / 1000000);
    return ERR_OK;
}

static FlintError nativeNanoTime(FlintExecution *exec) {
    exec->stackPushInt64(FlintAPI::System::getNanoTime());
    return ERR_OK;
}

static FlintError nativeArraycopy(FlintExecution *exec) {
    int32_t length = exec->stackPopInt32();
    int32_t destPos = exec->stackPopInt32();
    JObject *dest = exec->stackPopObject();
    int32_t srcPos = exec->stackPopInt32();
    JObject *src = exec->stackPopObject();
    if(src->dimensions == 0 || dest->dimensions == 0) {
        if(src->dimensions == 0)
            return throwArrayStoreException(exec, "Source object is not a array");
        else
            return throwArrayStoreException(exec, "Destination object is not a array");
    }
    else if(src->type == dest->type) {
        uint8_t atype = JObject::isPrimType(src->type);
        uint8_t elementSize = atype ? JObject::getPrimitiveTypeSize(atype) : sizeof(JObject *);
        if(length < 0)
            return throwArrayIndexOutOfBoundsException(exec, 0, length);
        if((length + srcPos) > src->size / elementSize)
            return throwArrayIndexOutOfBoundsException(exec, length + srcPos, src->size / elementSize);
        if((length + destPos) > dest->size / elementSize)
            return throwArrayIndexOutOfBoundsException(exec, length + destPos, dest->size / elementSize);
        void *srcVal = ((JInt8Array *)src)->getData();
        void *dstVal = ((JInt8Array *)dest)->getData();
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
    else
        return throwArrayStoreException(exec, "Type mismatch, can not copy array object");
    return ERR_OK;
}

static FlintError nativeIdentityHashCode(FlintExecution *exec) {
    JObject *obj = exec->stackPopObject();
    exec->stackPushInt32((int32_t)obj);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\xF9\xDF""currentTimeMillis", "\x03\x00\x90\x50""()J",                                        nativeCurrentTimeMillis),
    NATIVE_METHOD("\x08\x00\xB8\x86""nanoTime",          "\x03\x00\x90\x50""()J",                                        nativeNanoTime),
    NATIVE_METHOD("\x09\x00\xDB\xA7""arraycopy",         "\x2A\x00\xC2\xBC""(Ljava/lang/Object;ILjava/lang/Object;II)V", nativeArraycopy),
    NATIVE_METHOD("\x10\x00\xC5\xB8""identityHashCode",  "\x15\x00\x49\x76""(Ljava/lang/Object;)I",                      nativeIdentityHashCode),
};

const FlintNativeClass SYSTEM_CLASS = NATIVE_CLASS(systemClassName, methods);
