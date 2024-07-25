
#ifndef __MJVM_OBJECT_H
#define __MJVM_OBJECT_H

#include "mjvm_std_types.h"
#include "mjvm_const_pool.h"

class MjvmObject {
private:
    MjvmObject *next;
    MjvmObject *prev;
public:
    const uint32_t size : 30;
private:
    uint32_t prot : 2;
public:
    MjvmConstUtf8 &type;
    const uint32_t dimensions : 8;
    uint32_t monitorCount : 24;
    uint32_t ownId;
    uint8_t data[];

    static uint8_t getPrimitiveTypeSize(uint8_t atype);
    static uint8_t convertToAType(char type);
    static uint8_t isPrimType(const MjvmConstUtf8 &type);
private:
    uint8_t parseTypeSize(void) const;

    void setProtected(void);
    void clearProtected(void);
    uint8_t getProtected(void) const;
protected:
    MjvmObject(uint32_t size, MjvmConstUtf8 &type, uint8_t dimensions);
    MjvmObject(const MjvmObject &) = delete;
    void operator=(const MjvmObject &) = delete;

    friend class Mjvm;
    friend class MjvmExecution;
};

#endif /* __MJVM_OBJECT_H */
