
#ifndef __FLINT_JAVA_OBJECT_H
#define __FLINT_JAVA_OBJECT_H

#include "flint_std_types.h"
#include "flint_const_pool.h"

class FlintJavaObject {
private:
    FlintJavaObject *next;
    FlintJavaObject *prev;
public:
    const uint32_t size : 30;
private:
    uint32_t prot : 2;
public:
    FlintConstUtf8 &type;
    const uint32_t dimensions : 8;
private:
    uint32_t monitorCount : 24;
    uint32_t ownId;
protected:
    uint8_t data[];
public:
    static uint8_t getPrimitiveTypeSize(uint8_t atype);
    static uint8_t convertToAType(char type);
    static uint8_t isPrimType(const FlintConstUtf8 &type);

    uint8_t parseTypeSize(void) const;

    class FlintFieldsData &getFields(void) const;
private:
    void clearProtected(void);
    void setProtected(void);
    uint8_t getProtected(void) const;
protected:
    FlintJavaObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions);
    FlintJavaObject(const FlintJavaObject &) = delete;
    void operator=(const FlintJavaObject &) = delete;

    friend class Flint;
    friend class FlintExecution;
    friend class FlintDebugger;
};

#endif /* __FLINT_JAVA_OBJECT_H */
