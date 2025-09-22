
#ifndef __FLINT_JAVA_OBJECT_H
#define __FLINT_JAVA_OBJECT_H

#include "flint_std.h"
#include "flint_list.h"
#include "flint_fields_data.h"

class JObject : public ListNode {
protected:
    const uint32_t size : 30;
private:
    uint32_t prot : 2;
public:
    class JClass * const type;  /* NULL if JClass */
private:
    uint32_t monitorCount;
    uint32_t ownId;
protected:
    uint8_t data[];
public:
    const char *getTypeName(void) const;

    bool initFields(class FExec *ctx, class ClassLoader *loader);

    Field32 *getField32(class FExec *ctx, ConstField *field) const;
    Field32 *getField32(class FExec *ctx, const char *name) const;
    Field32 *getField32ByIndex(uint32_t index) const;

    Field64 *getField64(class FExec *ctx, ConstField *field) const;
    Field64 *getField64(class FExec *ctx, const char *name) const;
    Field64 *getField64ByIndex(uint32_t index) const;

    FieldObj *getFieldObj(class FExec *ctx, ConstField *field) const;
    FieldObj *getFieldObj(class FExec *ctx, const char *name) const;
    FieldObj *getFieldObjByIndex(uint32_t index) const;

    void clearData(void);

    bool isArray(void) const;

    void clearProtected(void);
    void setProtected(void);
    uint8_t getProtected(void) const;
protected:
    JObject(uint32_t size, class JClass *type);
    JObject(const JObject &) = delete;
    void operator=(const JObject &) = delete;

    ~JObject(void);

    friend class Flint;
    friend class FExec;
    friend class FDbg;
};

#endif /* __FLINT_JAVA_OBJECT_H */
