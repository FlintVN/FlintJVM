
#ifndef __MJVM_NATIVE_CLASS_H
#define __MJVM_NATIVE_CLASS_H

#include "mjvm_common.h"
#include "mjvm_execution.h"
#include "mjvm_const_pool.h"
#include "mjvm_attribute_info.h"

#define NATIVE_CLASS(_className, _methods) {                \
    .className = *(ConstUtf8 *)&_className,                 \
    .methodCount = LENGTH(_methods),                        \
    .methods = _methods                                     \
}

#define NATIVE_METHOD(_name, _descriptor, _nativeMathod) {  \
    .name = *(ConstUtf8 *)_name,                            \
    .descriptor = *(ConstUtf8 *)_descriptor,                \
    .nativeMathod = _nativeMathod                           \
}

class NativeMethod {
public:
    ConstUtf8 &name;
    ConstUtf8 &descriptor;
    NativeMethodPtr nativeMathod;
private:
    void operator=(const NativeMethod &) = delete;
};

class NativeClass {
public:
    ConstUtf8 &className;
    uint16_t methodCount;
    const NativeMethod *methods;
private:
    void operator=(const NativeClass &) = delete;
};

extern const NativeClass *NATIVE_CLASS_LIST[];
extern const uint32_t NATIVE_CLASS_COUNT;

#endif /* __MJVM_NATIVE_METHOD_H */
