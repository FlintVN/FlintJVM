
#ifndef __MJVM_NATIVE_CLASS_H
#define __MJVM_NATIVE_CLASS_H

#include "mjvm_common.h"
#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

#define NATIVE_CLASS(className, methods)                    {className, LENGTH(methods), methods}
#define NATIVE_METHOD(name, descriptor, nativeMathod)       {*(ConstUtf8 *)name, *(ConstUtf8 *)descriptor, nativeMathod}

class NativeMethod {
public:
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;
    NativeMethodPtr nativeMathod;
private:
    NativeMethod(void) = delete;
    NativeMethod(const NativeMethod &) = delete;
    void operator=(const NativeMethod &) = delete;
};

class NativeClass {
public:
    const ConstUtf8 &className;
    const uint16_t methodCount;
    const NativeMethod *methods;
private:
    NativeClass(void) = delete;
    NativeClass(const NativeClass &) = delete;
    void operator=(const NativeClass &) = delete;
};      

extern const NativeClass *NATIVE_CLASS_LIST[];
extern const uint32_t NATIVE_CLASS_COUNT;

#endif /* __MJVM_NATIVE_METHOD_H */
