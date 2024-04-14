
#ifndef __MJVM_OBJECT_H
#define __MJVM_OBJECT_H

#include "mjvm_std_types.h"
#include "mjvm_const_pool.h"

class MjvmObject {
public:
    const uint32_t size : 31;
private:
    uint32_t prot : 1;
public:
    const ConstUtf8 &type;
    const uint32_t dimensions : 8;
public:
    uint32_t monitorCount : 24;
    uint32_t ownId;
protected:
    uint8_t data[];
private:
    uint8_t parseTypeSize(void) const;

    void setProtected(void);
    bool isProtected(void) const;
private:
    MjvmObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions);
    MjvmObject(const MjvmObject &) = delete;
    void operator=(const MjvmObject &) = delete;

    friend class Execution;
};

class MjvmString : public MjvmObject {
public:
    const char *getText(void) const;
    uint32_t getLength(void) const;
protected:
    MjvmString(void) = delete;
    MjvmString(const MjvmString &) = delete;
    void operator=(const MjvmString &) = delete;
};

class MjvmThrowable : public MjvmObject {
public:
    MjvmString &getDetailMessage(void) const;
protected:
    MjvmThrowable(void) = delete;
    MjvmThrowable(const MjvmThrowable &) = delete;
    void operator=(const MjvmThrowable &) = delete;
};

class MjvmObjectNode {
public:
    MjvmObjectNode *prev;
    MjvmObjectNode *next;

    MjvmObject *getMjvmObject(void) const;
private:
    uint8_t data[];
};

#endif /* __MJVM_OBJECT_H */
