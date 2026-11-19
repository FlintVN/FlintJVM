
#ifndef __FLINT_METHOD_INFO_H
#define __FLINT_METHOD_INFO_H

#include "flint_std.h"
#include "flint_native.h"
#include "flint_const_pool.h"

class Exceptions {
public:
    const uint16_t length;
    const uint16_t exceptionTypes[];
private:
    Exceptions(void) = delete;
    Exceptions(const Exceptions &) = delete;
    void operator=(const Exceptions &) = delete;

    friend class ClassLoader;
};

class ExceptionEntry {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const uint16_t catchType;
private:
    ExceptionEntry(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType);
    ExceptionEntry(const ExceptionEntry &) = delete;
    void operator=(const ExceptionEntry &) = delete;

    friend class ClassLoader;
};

class CodeAttribute {
private:
    uint16_t maxStack;
    uint16_t maxLocals;
    uint32_t codeLength;
    uint16_t exceptionLength;
    uint8_t data[];

    CodeAttribute(const CodeAttribute &) = delete;
    void operator=(const CodeAttribute &) = delete;

    friend class MethodInfo;
    friend class ClassLoader;
};

typedef enum : uint16_t {
    METHOD_PUBLIC = 0x0001,
    METHOD_PRIVATE = 0x0002,
    METHOD_PROTECTED = 0x0004,
    METHOD_STATIC = 0x0008,
    METHOD_FINAL = 0x0010,
    METHOD_SYNCHRONIZED = 0x0020,
    METHOD_BRIDGE = 0x0040,
    METHOD_VARARGS = 0x0080,
    METHOD_NATIVE = 0x0100,
    METHOD_ABSTRACT = 0x0400,
    METHOD_STRICT = 0x0800,
    METHOD_SYNTHETIC = 0x1000,

    METHOD_UNLOADED = 0x2000,
    METHOD_INIT = 0x4000,
    METHOD_CLINIT = 0x8000,
} MethodAccessFlag;

class MethodInfo {
public:
    MethodAccessFlag accessFlag;
    class ClassLoader * const loader;
    union {
        struct {
            const char * const name;
            const char * const desc;
            const uint32_t hash;
        };
        ConstNameAndType nameAndType;
    };
private:
    Exceptions *exceptions;
    const char * retType;
    uint8_t *code;
public:
    inline __attribute__((always_inline)) const char *getReturnType(void) {
        if (retType == NULL) {
            const char *tmp = desc;
            while (*tmp != ')')
                tmp++;
            retType = ++tmp;
        }
        return retType;
    }

    inline __attribute__((always_inline)) uint8_t *getCode(void) {
        if (accessFlag & METHOD_NATIVE) {
            if (code == 0)
                code = (uint8_t *)NativeClass::findNativeMethod(this);
            return (uint8_t *)code;
        }
        CodeAttribute *codeAttr = (CodeAttribute *)code;
        return (uint8_t *)&((ExceptionEntry *)codeAttr->data)[codeAttr->exceptionLength];
    }

    inline __attribute__((always_inline)) uint32_t getCodeLength(void) const {
        return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->codeLength;
    }

    inline __attribute__((always_inline)) uint16_t getMaxLocals(void) const {
        return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->maxLocals;
    }

    inline __attribute__((always_inline)) uint16_t getMaxStack(void) const {
        return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->maxStack;
    }

    inline __attribute__((always_inline)) Exceptions *getExceptions(void) const {
        return exceptions;
    }

    inline __attribute__((always_inline)) uint16_t getExceptionEntryLength(void) const {
        return (accessFlag & METHOD_NATIVE) ? 0 : ((CodeAttribute *)code)->exceptionLength;
    }

    inline __attribute__((always_inline)) ExceptionEntry *getExceptionEntry(uint16_t index) const {
        if (accessFlag & METHOD_NATIVE)
            return NULL;
        CodeAttribute *codeAttr = (CodeAttribute *)code;
        return &((ExceptionEntry *)codeAttr->data)[index];
    }
private:
    MethodInfo(ClassLoader *loader, MethodAccessFlag accessFlag, const char *name, const char *desc);
    MethodInfo(const MethodInfo &) = delete;
    void operator=(const MethodInfo &) = delete;

    friend class ClassLoader;
};

#endif /* __FLINT_METHOD_INFO_H */
