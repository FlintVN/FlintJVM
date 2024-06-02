
#ifndef __MJVM_CONST_POOL_H
#define __MJVM_CONST_POOL_H

#include "mjvm_types.h"

class ConstPool {
public:
    volatile const ConstPoolTag tag;
    volatile const uint32_t value;
private:
    ConstPool(void) = delete;
    ConstPool(const ConstPool &) = delete;
    void operator=(const ConstPool &) = delete;
};

class ConstUtf8 {
public:
    const uint16_t length;
    const char text[];
private:
    ConstUtf8(const ConstUtf8 &) = delete;
    void operator=(const ConstUtf8 &) = delete;

    friend class ClassLoader;
public:
    bool operator==(const ConstUtf8 &another) const;
    bool operator!=(const ConstUtf8 &another) const;
};

class ConstNameAndType {
public:
    ConstUtf8 &name;
    ConstUtf8 &descriptor;

    bool operator==(const ConstNameAndType &another) const;
    bool operator!=(const ConstNameAndType &another) const;
private:
    ConstNameAndType(ConstUtf8 &name, ConstUtf8 &descriptor);
    ConstNameAndType(const ConstNameAndType &) = delete;
    void operator=(const ConstNameAndType &) = delete;

    friend class ClassLoader;
};

class ConstField {
public:
    ConstUtf8 &className;
    ConstNameAndType &nameAndType;
private:
    ConstField(ConstUtf8 &className, ConstNameAndType &nameAndType);
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
    ConstUtf8 &className;
    ConstNameAndType &nameAndType;

    ParamInfo parseParamInfo(void) const;
private:
    ConstMethod(ConstUtf8 &className, ConstNameAndType &nameAndType);
    ConstMethod(const ConstMethod &) = delete;
    void operator=(const ConstMethod &) = delete;

    friend class ClassLoader;
};

typedef ConstMethod ConstInterfaceMethod;

#endif /* __MJVM_CONST_POOL_H */
