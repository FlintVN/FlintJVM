
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
private:
    uint8_t data[];

    uint8_t parseTypeSize(void) const;

    void setProtected(void);
    bool isProtected(void) const;
private:
    MjvmObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions);
    MjvmObject(const MjvmObject &) = delete;
    void operator=(const MjvmObject &) = delete;

    friend class Execution;
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
