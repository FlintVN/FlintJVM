#ifndef __FLINT_FIELDS_DATA_H
#define __FLINT_FIELDS_DATA_H

#include "flint_const_pool.h"
#include "flint_field_info.h"

class FieldValue {
private:
    const FieldInfo * const fieldInfo;
    uint32_t value;
public:
    const FieldInfo *getFieldInfo(void) const;
    int32_t getInt32(void) const;
    class JObject *getObj(void) const;
    int64_t getInt64(void) const;
    void setInt32(int32_t val);
    void setObj(class JObject *obj);
    void setInt64(int64_t val);
private:
    FieldValue(const FieldInfo *fieldInfo);
    FieldValue(const FieldValue &) = delete;
    void operator=(const FieldValue &) = delete;

    friend class FieldsData;
};

class FieldsData {
private:
    uint16_t count;
    uint16_t objCount;

    FieldValue *fields;
public:
    FieldsData(void);

    uint16_t hasObjField(void) const;

    FieldValue *getField(ConstField *field) const;
    FieldValue *getField(const char *name) const;
    FieldValue *getFieldByIndex(uint32_t index) const;

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
