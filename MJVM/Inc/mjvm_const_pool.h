
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
    const char text[];
private:
    ConstUtf8(void) = delete;
    ConstUtf8(const ConstUtf8 &) = delete;
    void operator=(const ConstUtf8 &) = delete;
};

class ConstNameAndType {
public:
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;

    ConstNameAndType(const ConstUtf8 &name, const ConstUtf8 &descriptor);
private:
    ConstNameAndType(const ConstNameAndType &) = delete;
    void operator=(const ConstNameAndType &) = delete;
};

class ConstField {
public:
    const ConstUtf8 &className;
    const ConstNameAndType &nameAndType;

    ConstField(const ConstUtf8 &className, const ConstNameAndType &nameAndType);
private:
    ConstField(const ConstField &) = delete;
    void operator=(const ConstField &) = delete;
};

typedef ConstField ConstMethod;
typedef ConstField ConstInterfaceMethod;

#endif /* __MJVM_CONST_POOL_H */
