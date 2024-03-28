
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
};

class ConstNameAndType {
public:
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;
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

typedef ConstField ConstMethod;
typedef ConstField ConstInterfaceMethod;

#endif /* __MJVM_CONST_POOL_H */
