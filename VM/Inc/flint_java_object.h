
#ifndef __FLINT_JAVA_OBJECT_H
#define __FLINT_JAVA_OBJECT_H

#include "flint_std_types.h"
#include "flint_const_pool.h"

class JObject {
private:
    JObject *next;
    JObject *prev;
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
    static uint8_t isPrimType(FlintConstUtf8 &type);

    uint8_t parseTypeSize(void) const;

    class FlintFieldsData &getFields(void) const;

    void clearProtected(void);
    void setProtected(void);
    uint8_t getProtected(void) const;
protected:
    JObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions);
    JObject(const JObject &) = delete;
    void operator=(const JObject &) = delete;

    friend class Flint;
    friend class FlintExecution;
    friend class FlintDebugger;
};

#endif /* __FLINT_JAVA_OBJECT_H */
