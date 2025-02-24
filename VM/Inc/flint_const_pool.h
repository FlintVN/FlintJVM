
#ifndef __FLINT_CONST_POOL_H
#define __FLINT_CONST_POOL_H

#include "flint_types.h"

#define CONST_UTF8_HASH(utf8)       *(uint32_t *)&(utf8)

class FlintConstPool {
public:
    volatile const FlintConstPoolTag tag;
    volatile const uint32_t value;
private:
    FlintConstPool(void) = delete;
    FlintConstPool(const FlintConstPool &) = delete;
    void operator=(const FlintConstPool &) = delete;
};

class FlintConstUtf8 {
public:
    const uint16_t length;
    const uint16_t crc;
    const char text[];
private:
    FlintConstUtf8(const FlintConstUtf8 &) = delete;
    void operator=(const FlintConstUtf8 &) = delete;

    friend class FlintClassLoader;
public:
    bool operator==(const FlintConstUtf8 &another) const;
    bool operator!=(const FlintConstUtf8 &another) const;
};

class FlintConstNameAndType {
public:
    FlintConstUtf8 &name;
    FlintConstUtf8 &descriptor;

    bool operator==(const FlintConstNameAndType &another) const;
    bool operator!=(const FlintConstNameAndType &another) const;
private:
    FlintConstNameAndType(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor);
    FlintConstNameAndType(const FlintConstNameAndType &) = delete;
    void operator=(const FlintConstNameAndType &) = delete;

    friend class FlintClassLoader;
};

class FlintConstField {
public:
    FlintConstUtf8 &className;
    FlintConstNameAndType &nameAndType;
private:
    uint32_t fieldIndex;
private:
    FlintConstField(const FlintConstUtf8 &className, FlintConstNameAndType &nameAndType);
    FlintConstField(const FlintConstField &) = delete;
    void operator=(const FlintConstField &) = delete;

    friend class FlintClassLoader;
    friend class FlintFieldsData;
};

typedef struct {
    uint8_t argc;
    uint8_t retType;
} FlintParamInfo;

class FlintConstMethod {
public:
    FlintConstUtf8 &className;
    FlintConstNameAndType &nameAndType;
private:
    class FlintMethodInfo *methodInfo;
    FlintParamInfo paramInfo;
public:
    const FlintParamInfo &getParmInfo(void);
private:
    FlintConstMethod(const FlintConstUtf8 &className, FlintConstNameAndType &nameAndType);
    FlintConstMethod(const FlintConstUtf8 &className, FlintConstNameAndType &nameAndType, uint8_t argc, uint8_t retType);
    FlintConstMethod(const FlintConstMethod &) = delete;
    void operator=(const FlintConstMethod &) = delete;

    friend class Flint;
    friend class FlintExecution;
    friend class FlintClassLoader;
};

typedef FlintConstMethod FlintConstInterfaceMethod;

#endif /* __FLINT_CONST_POOL_H */
