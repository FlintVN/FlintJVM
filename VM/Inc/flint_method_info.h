
#ifndef __FLINT_METHOD_INFO_H
#define __FLINT_METHOD_INFO_H

#include "flint_std.h"
#include "flint_const_pool.h"

class ExceptionTable {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const uint16_t catchType;
private:
    ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType);
    ExceptionTable(const ExceptionTable &) = delete;
    void operator=(const ExceptionTable &) = delete;

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
    const char * const name;
    const char * const desc;
    const uint32_t hash;
private:
    const char * retType;
    uint8_t *code;
public:
    const char *getReturnType(void);
    uint8_t *getCode(void);
    uint32_t getCodeLength(void) const;
    uint16_t getMaxLocals(void) const;
    uint16_t getMaxStack(void) const;
    uint16_t getExceptionLength(void) const;
    ExceptionTable *getException(uint16_t index) const;
private:
    MethodInfo(ClassLoader *loader, MethodAccessFlag accessFlag, const char *name, const char *desc);
    MethodInfo(const MethodInfo &) = delete;
    void operator=(const MethodInfo &) = delete;

    friend class ClassLoader;
};

#endif /* __FLINT_METHOD_INFO_H */
