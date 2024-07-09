
#ifndef __MJVM_CONST_POOL_H
#define __MJVM_CONST_POOL_H

#include "mjvm_types.h"

#define CONST_UTF8_HASH(utf8)       *(uint32_t *)&(utf8)

class MjvmConstPool {
public:
    volatile const MjvmConstPoolTag tag;
    volatile const uint32_t value;
private:
    MjvmConstPool(void) = delete;
    MjvmConstPool(const MjvmConstPool &) = delete;
    void operator=(const MjvmConstPool &) = delete;
};

class MjvmConstUtf8 {
public:
    const uint16_t length;
    const uint16_t crc;
    const char text[];
private:
    MjvmConstUtf8(const MjvmConstUtf8 &) = delete;
    void operator=(const MjvmConstUtf8 &) = delete;

    friend class MjvmClassLoader;
public:
    bool operator==(const MjvmConstUtf8 &another) const;
    bool operator!=(const MjvmConstUtf8 &another) const;
};

class MjvmConstNameAndType {
public:
    MjvmConstUtf8 &name;
    MjvmConstUtf8 &descriptor;

    bool operator==(const MjvmConstNameAndType &another) const;
    bool operator!=(const MjvmConstNameAndType &another) const;
private:
    MjvmConstNameAndType(MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor);
    MjvmConstNameAndType(const MjvmConstNameAndType &) = delete;
    void operator=(const MjvmConstNameAndType &) = delete;

    friend class MjvmClassLoader;
};

class MjvmConstField {
public:
    MjvmConstUtf8 &className;
    MjvmConstNameAndType &nameAndType;
private:
    MjvmConstField(MjvmConstUtf8 &className, MjvmConstNameAndType &nameAndType);
    MjvmConstField(const MjvmConstField &) = delete;
    void operator=(const MjvmConstField &) = delete;

    friend class MjvmClassLoader;
};

typedef struct {
    uint8_t argc;
    uint8_t retType;
} MjvmParamInfo;

class MjvmConstMethod {
public:
    MjvmConstUtf8 &className;
    MjvmConstNameAndType &nameAndType;
private:
    MjvmParamInfo paramInfo;
public:
    const MjvmParamInfo &getParmInfo(void);
private:
    MjvmConstMethod(MjvmConstUtf8 &className, MjvmConstNameAndType &nameAndType);
    MjvmConstMethod(MjvmConstUtf8 &className, MjvmConstNameAndType &nameAndType, uint8_t argc, uint8_t retType);
    MjvmConstMethod(const MjvmConstMethod &) = delete;
    void operator=(const MjvmConstMethod &) = delete;

    friend class MjvmClassLoader;
    friend class MjvmExecution;
};

typedef MjvmConstMethod MjvmConstInterfaceMethod;

#endif /* __MJVM_CONST_POOL_H */
