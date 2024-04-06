
#ifndef __MJVM_CONST_POOL_H
#define __MJVM_CONST_POOL_H

#include "mjvm_types.h"

class ConstPool {
public:
    const ConstPoolTag tag;
    const uint32_t value;
private:
    ConstPool(void) = delete;
    ConstPool(const ConstPool &) = delete;
    void operator=(const ConstPool &) = delete;
};

class ConstUtf8 {
public:
    const uint16_t length;
private:
    char text[];

    ConstUtf8(uint16_t length);
    ConstUtf8(const ConstUtf8 &) = delete;
    void operator=(const ConstUtf8 &) = delete;

    friend class ClassLoader;
public:
    const char *getText(void) const;

    bool operator==(const ConstUtf8 &another) const;
    bool operator!=(const ConstUtf8 &another) const;
};

class ConstNameAndType {
public:
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;

    bool operator==(const ConstNameAndType &another) const;
    bool operator!=(const ConstNameAndType &another) const;
private:
    ConstNameAndType(const ConstUtf8 &name, const ConstUtf8 &descriptor);
    ConstNameAndType(const ConstNameAndType &) = delete;
    void operator=(const ConstNameAndType &) = delete;

    friend class ClassLoader;
};

class ConstField {
public:
    const ConstUtf8 &className;
    const ConstNameAndType &nameAndType;
private:
    ConstField(const ConstUtf8 &className, const ConstNameAndType &nameAndType);
    ConstField(const ConstField &) = delete;
    void operator=(const ConstField &) = delete;

    friend class ClassLoader;
};

typedef struct {
    uint8_t argc;
    uint8_t retType;
} ParamInfo;

ParamInfo parseParamInfo(const ConstUtf8 &descriptor);

class ConstMethod {
public:
    const ConstUtf8 &className;
    const ConstNameAndType &nameAndType;

    ParamInfo parseParamInfo(void) const;
private:
    ConstMethod(const ConstUtf8 &className, const ConstNameAndType &nameAndType);
    ConstMethod(const ConstMethod &) = delete;
    void operator=(const ConstMethod &) = delete;

    friend class ClassLoader;
};

typedef ConstMethod ConstInterfaceMethod;

#endif /* __MJVM_CONST_POOL_H */
