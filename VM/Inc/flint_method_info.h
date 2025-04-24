
#ifndef __FLINT_METHOD_INFO_H
#define __FLINT_METHOD_INFO_H

#include "flint_const_pool.h"

class FlintExceptionTable {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const uint16_t catchType;
private:
    FlintExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType);
    FlintExceptionTable(const FlintExceptionTable &) = delete;
    void operator=(const FlintExceptionTable &) = delete;

    friend class FlintClassLoader;
};

class FlintCodeAttribute {
private:
    uint16_t maxStack;
    uint16_t maxLocals;
    uint32_t codeLength;
    uint16_t exceptionLength;
    uint8_t data[];

    FlintCodeAttribute(const FlintCodeAttribute &) = delete;
    void operator=(const FlintCodeAttribute &) = delete;

    friend class FlintMethodInfo;
    friend class FlintClassLoader;
};

class FlintMethodInfo {
public:
    FlintMethodAccessFlag accessFlag;
    class FlintClassLoader &classLoader;
private:
    uint16_t nameIndex;
    uint16_t descIndex;
private:
    uint8_t *code;

    FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, uint16_t nameIndex, uint16_t descIndex);

    FlintMethodInfo(const FlintMethodInfo &) = delete;
    void operator=(const FlintMethodInfo &) = delete;

    friend class FlintClassLoader;
public:
    FlintConstUtf8 &getName(void) const;
    FlintConstUtf8 &getDescriptor(void) const;

    uint8_t *getCode(void);
    uint32_t getCodeLength(void) const;
    uint16_t getMaxLocals(void) const;
    uint16_t getMaxStack(void) const;
    uint16_t getExceptionLength(void) const;
    FlintExceptionTable *getException(uint16_t index) const;

    bool isStaticCtor(void);

    ~FlintMethodInfo(void);
};

#endif /* __FLINT_METHOD_INFO_H */
