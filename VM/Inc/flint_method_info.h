
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

class FlintMethodInfo {
public:
    class FlintClassLoader &classLoader;
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
    const FlintMethodAccessFlag accessFlag;
private:
    uint16_t maxStack;
    uint8_t *code;
    uint16_t maxLocals;
    uint16_t exceptionLength;
    uint32_t codeLength;
    FlintExceptionTable *exceptionTable;

    FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor);

    FlintMethodInfo(const FlintMethodInfo &) = delete;
    void operator=(const FlintMethodInfo &) = delete;

    void setCode(uint8_t *code, uint32_t codeLength, uint16_t maxStack, uint16_t maxLocals);
    void setException(FlintExceptionTable *exceptionTable, uint16_t exceptionLength);

    friend class FlintClassLoader;
public:
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
