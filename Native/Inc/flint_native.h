
#ifndef __FLINT_NATIVE_METHODS_H
#define __FLINT_NATIVE_METHODS_H

#include "flint_std.h"
#include "flint_common.h"
#include "flint_native_interface.h"

#define NATIVE_CLASS(name, methods)         NativeClass(name, methods, LENGTH(methods))
#define NATIVE_METHOD(name, desc, method)   NativeMethod(name, desc, (uint32_t)method)

typedef void (*JNMPtr)(FNIEnv *env, ...);

class NativeMethod {
public:
    const char * const name;
    const char * const desc;
    const uint32_t hash;
    const uint32_t methodPtr;

    constexpr NativeMethod(const char *name, const char *desc, uint32_t method) :
    name(name), desc(desc),
    hash((Hash(name) & 0xFFFF) | (Hash(desc) << 16)),
    methodPtr(method) { }
private:
    void operator=(const NativeMethod &) = delete;

    friend class NativeClass;
};

class NativeClass {
public:
    const char * const className;
    const NativeMethod * const methods;
    const uint32_t hash;
    const uint16_t methodCount;

    consteval NativeClass(const char *className, const NativeMethod *methods, uint32_t count) :
    className(className), methods(methods), hash(Hash(className)), methodCount(count) {

    }

    static JNMPtr findNativeMethod(class MethodInfo *methodInfo);
private:
    void operator=(const NativeClass &) = delete;
};

#endif /* __FLINT_NATIVE_METHODS_H */
