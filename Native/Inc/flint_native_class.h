
#ifndef __FLINT_NATIVE_CLASS_H
#define __FLINT_NATIVE_CLASS_H

#include "flint_common.h"
#include "flint_execution.h"
#include "flint_const_pool.h"
#include "flint_attribute_info.h"

#define NATIVE_CLASS(_className, _methods) {                \
    .className = *(FlintConstUtf8 *)&_className,            \
    .methodCount = LENGTH(_methods),                        \
    .methods = _methods                                     \
}

#define NATIVE_METHOD(_name, _descriptor, _nativeMathod) {  \
    .name = *(FlintConstUtf8 *)_name,                       \
    .descriptor = *(FlintConstUtf8 *)_descriptor,           \
    .nativeMathod = _nativeMathod                           \
}

class FlintNativeMethod {
public:
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
    FlintNativeMethodPtr nativeMathod;
private:
    void operator=(const FlintNativeMethod &) = delete;
};

class FlintNativeClass {
public:
    FlintConstUtf8 &className;
    uint16_t methodCount;
    const FlintNativeMethod *methods;
private:
    void operator=(const FlintNativeClass &) = delete;
};

extern const FlintNativeClass *BASE_NATIVE_CLASS_LIST[13];

#endif /* __FLINT_NATIVE_METHOD_H */
