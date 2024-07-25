
#ifndef __FLINT_OBJECT_H
#define __FLINT_OBJECT_H

#include "flint_std_types.h"
#include "flint_const_pool.h"

class FlintObject {
private:
    FlintObject *next;
    FlintObject *prev;
public:
    const uint32_t size : 30;
private:
    uint32_t prot : 2;
public:
    FlintConstUtf8 &type;
    const uint32_t dimensions : 8;
    uint32_t monitorCount : 24;
    uint32_t ownId;
    uint8_t data[];

    static uint8_t getPrimitiveTypeSize(uint8_t atype);
    static uint8_t convertToAType(char type);
    static uint8_t isPrimType(const FlintConstUtf8 &type);
private:
    uint8_t parseTypeSize(void) const;

    void setProtected(void);
    void clearProtected(void);
    uint8_t getProtected(void) const;
protected:
    FlintObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions);
    FlintObject(const FlintObject &) = delete;
    void operator=(const FlintObject &) = delete;

    friend class Flint;
    friend class FlintExecution;
};

#endif /* __FLINT_OBJECT_H */
