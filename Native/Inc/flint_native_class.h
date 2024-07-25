
#ifndef __FLINT_NATIVE_CLASS_H
#define __FLINT_NATIVE_CLASS_H

#include "flint_common.h"
#include "flint_execution.h"
#include "flint_const_pool.h"
#include "flint_attribute_info.h"

#define NATIVE_CLASS(_className, _methods) {                \
    .className = *(FlintConstUtf8 *)&_className,                 \
    .methodCount = LENGTH(_methods),                        \
    .methods = _methods                                     \
}

#define NATIVE_METHOD(_name, _descriptor, _nativeMathod) {  \
    .name = *(FlintConstUtf8 *)_name,                            \
    .descriptor = *(FlintConstUtf8 *)_descriptor,                \
    .nativeMathod = _nativeMathod                           \
}

class NativeMethod {
public:
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
    FlintNativeMethodPtr nativeMathod;
private:
    void operator=(const NativeMethod &) = delete;
};

class NativeClass {
public:
    FlintConstUtf8 &className;
    uint16_t methodCount;
    const NativeMethod *methods;
private:
    void operator=(const NativeClass &) = delete;
};

extern const NativeClass *NATIVE_CLASS_LIST[];
extern const uint32_t NATIVE_CLASS_COUNT;

#endif /* __FLINT_NATIVE_METHOD_H */
