
#ifndef __FLINT_CONST_POOL_H
#define __FLINT_CONST_POOL_H

#include "flint_common.h"

typedef enum : uint8_t {
    CONST_UTF8 = 1,
    CONST_INTEGER = 3,
    CONST_FLOAT = 4,
    CONST_LONG = 5,
    CONST_DOUBLE = 6,
    CONST_CLASS = 7,
    CONST_STRING = 8,
    CONST_FIELD = 9,
    CONST_METHOD = 10,
    CONST_INTERFACE_METHOD = 11,
    CONST_NAME_AND_TYPE = 12,
    CONST_METHOD_HANDLE = 15,
    CONST_METHOD_TYPE = 16,
    CONST_INVOKE_DYNAMIC = 18,
    CONST_UNKOWN = 0xFF,
} ConstPoolTag;

class ConstPool {
public:
    const ConstPoolTag tag;
    const uint32_t value;
private:
    ConstPool(void) = delete;
    ConstPool(const ConstPool &) = delete;
    void operator=(const ConstPool &) = delete;
};

class ConstNameAndType {
public:
    const char * const name;
    const char * const desc;
    const uint32_t hash;

    bool operator==(ConstNameAndType &another) const;
    bool operator!=(ConstNameAndType &another) const;

    constexpr ConstNameAndType(const char *name, const char *desc) :
    name(name), desc(desc),
    hash((Hash(name) & 0xFFFF) | (Hash(desc) << 16)) { }

    ConstNameAndType(const ConstNameAndType &) = delete;
    void operator=(const ConstNameAndType &) = delete;

    friend class ClassLoader;
};

class ConstField {
public:
    const char * const className;
    class ClassLoader *loader;
    ConstNameAndType *nameAndType;
private:
    uint32_t fieldIndex;
private:
    ConstField(const char *className, ConstNameAndType *nameAndType);
    ConstField(const ConstField &) = delete;
    void operator=(const ConstField &) = delete;

    friend class ClassLoader;
    friend class FieldsData;
};

class ConstMethod {
public:
    const char * const className;
    ConstNameAndType *nameAndType;
private:
    class MethodInfo *methodInfo;
    uint8_t argc;
public:
    uint8_t getArgc(void) const;
private:
    ConstMethod(const char *className, ConstNameAndType *nameAndType);
    ConstMethod(const ConstMethod &) = delete;
    void operator=(const ConstMethod &) = delete;

    friend class Flint;
    friend class FExec;
    friend class ClassLoader;
};

typedef ConstMethod ConstInterfaceMethod;

#endif /* __FLINT_CONST_POOL_H */
