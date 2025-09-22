#ifndef __FLINT_FIELDS_DATA_H
#define __FLINT_FIELDS_DATA_H

#include "flint_const_pool.h"
#include "flint_field_info.h"

class Field32 {
public:
    const FieldInfo * const fieldInfo;
    int32_t value;
private:
    Field32(const FieldInfo *fieldInfo);
    Field32(const Field32 &) = delete;
    void operator=(const Field32 &) = delete;

    friend class FieldsData;
};

class Field64 {
public:
    const FieldInfo *fieldInfo;
    int64_t value;
private:
    Field64(const FieldInfo *fieldInfo);
    Field64(const Field64 &) = delete;
    void operator=(const Field64 &) = delete;

    friend class FieldsData;
};

class FieldObj {
public:
    const FieldInfo *fieldInfo;
    class JObject *value;
private:
    FieldObj(const FieldInfo *fieldInfo);
    FieldObj(const FieldObj &) = delete;
    void operator=(const FieldObj &) = delete;

    friend class FieldsData;
};

class FieldsData {
private:
    uint16_t fields32Count;
    uint16_t fields64Count;
    uint16_t fieldsObjCount;

    Field32 *fields32;
    Field64 *fields64;
    FieldObj *fieldsObj;
public:
    FieldsData(void);

    Field32 *getField32(ConstField *field) const;
    Field32 *getField32(const char *name) const;
    Field32 *getField32ByIndex(uint32_t index) const;

    Field64 *getField64(ConstField *field) const;
    Field64 *getField64(const char *name) const;
    Field64 *getField64ByIndex(uint32_t index) const;

    FieldObj *getFieldObj(ConstField *field) const;
    FieldObj *getFieldObj(const char *name) const;
    FieldObj *getFieldObjByIndex(uint32_t index) const;

    bool init(class FExec *ctx, class ClassLoader *loader, bool isStatic);

    ~FieldsData(void);
private:
    bool initStatic(class FExec *ctx, class ClassLoader *loader);
    bool initNonStatic(class FExec *ctx, class ClassLoader *loader);
private:
    FieldsData(const FieldsData &) = delete;
    void operator=(const FieldsData &) = delete;

    friend class Flint;
};

#endif /* __FLINT_FIELDS_DATA_H */
