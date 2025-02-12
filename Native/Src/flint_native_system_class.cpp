
#include "flint.h"
#include "flint_java_object.h"
#include "flint_system_api.h"
#include "flint_const_name.h"
#include "flint_native_system_class.h"

static void nativeCurrentTimeMillis(FlintExecution &execution) {
    execution.stackPushInt64(FlintAPI::System::getNanoTime() / 1000000);
}

static void nativeNanoTime(FlintExecution &execution) {
    execution.stackPushInt64(FlintAPI::System::getNanoTime());
}

static void nativeArraycopy(FlintExecution &execution) {
    int32_t length = execution.stackPopInt32();
    int32_t destPos = execution.stackPopInt32();
    FlintJavaObject *dest = execution.stackPopObject();
    int32_t srcPos = execution.stackPopInt32();
    FlintJavaObject *src = execution.stackPopObject();
    if(src->dimensions == 0 || dest->dimensions == 0) {
        FlintJavaString *strObj;
        if(src->dimensions == 0)
            strObj = &execution.flint.newString(STR_AND_SIZE("Source object is not a array"));
        else
            strObj = &execution.flint.newString(STR_AND_SIZE("Destination object is not a array"));
        throw &execution.flint.newArrayStoreException(strObj);
    }
    else if(src->type == dest->type) {
        uint8_t atype = FlintJavaObject::isPrimType(src->type);
        uint8_t elementSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
        if((length < 0) || ((length + srcPos) > src->size / elementSize) || ((length + destPos) > dest->size / elementSize))
            throw "Index out of range in System.arraycopy";
        void *srcVal = ((FlintInt8Array *)src)->getData();
        void *dstVal = ((FlintInt8Array *)dest)->getData();
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
        FlintJavaString &strObj = execution.flint.newString(STR_AND_SIZE("Type mismatch, can not copy array object"));
        throw &execution.flint.newArrayStoreException(&strObj);
    }
}

static void nativeIdentityHashCode(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    execution.stackPushInt32((int32_t)obj);
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\xF9\xDF""currentTimeMillis", "\x03\x00\x90\x50""()J",                                        nativeCurrentTimeMillis),
    NATIVE_METHOD("\x08\x00\xB8\x86""nanoTime",          "\x03\x00\x90\x50""()J",                                        nativeNanoTime),
    NATIVE_METHOD("\x09\x00\xDB\xA7""arraycopy",         "\x2A\x00\xC2\xBC""(Ljava/lang/Object;ILjava/lang/Object;II)V", nativeArraycopy),
    NATIVE_METHOD("\x10\x00\xC5\xB8""identityHashCode",  "\x15\x00\x49\x76""(Ljava/lang/Object;)I",                      nativeIdentityHashCode),
};

const FlintNativeClass SYSTEM_CLASS = NATIVE_CLASS(systemClassName, methods);
