
#ifndef __FLINT_METHOD_INFO_H
#define __FLINT_METHOD_INFO_H

#include "flint_const_pool.h"
#include "flint_attribute_info.h"

class FlintMethodInfo {
public:
    const FlintMethodAccessFlag accessFlag;
    class FlintClassLoader &classLoader;
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;
private:
    void *code;

    FlintMethodInfo(FlintClassLoader &classLoader, FlintMethodAccessFlag accessFlag, const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor);

    FlintMethodInfo(const FlintMethodInfo &) = delete;
    void operator=(const FlintMethodInfo &) = delete;

    void setCode(FlintAttribute *attributeCode);

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
