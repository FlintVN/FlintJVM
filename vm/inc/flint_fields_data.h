
#ifndef __FLINT_FIELDS_DATA_H
#define __FLINT_FIELDS_DATA_H

#include "flint_const_pool.h"
#include "flint_field_info.h"

class FieldValue {
private:
    const FieldInfo * const fieldInfo;
    uint32_t value;
public:
    inline __attribute__((always_inline)) const FieldInfo *getFieldInfo(void) const {
        return fieldInfo;
    }

    inline __attribute__((always_inline)) int32_t getInt32(void) const {
        return (int32_t)value;
    }

    inline __attribute__((always_inline)) class JObject *getObj(void) const {
        return (class JObject *)value;
    }

    inline __attribute__((always_inline)) int64_t getInt64(void) const {
        return ((uint64_t)this[1].value << 32) | this[0].value;
    }

    inline __attribute__((always_inline)) void setInt32(int32_t val) {
        value = (uint32_t)val;
    }

    inline __attribute__((always_inline)) void setObj(JObject *obj) {
        value = (uint32_t)obj;
    }

    inline __attribute__((always_inline)) void setInt64(int64_t val) {
        this[0].value = (uint32_t)val;
        this[1].value = (uint64_t)val >> 32;
    }
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
    uint16_t getFieldsCount(void) const;

    FieldValue *getField(ConstField *field) const;
    FieldValue *getField(const char *name) const;
    FieldValue *getFieldByIndex(uint32_t index) const;

    bool init(class Flint *flint, class FExec *ctx, class ClassLoader *loader, bool isStatic);

    void destroy(class Flint *flint);
private:
    bool initStatic(class Flint *flint, class FExec *ctx, class ClassLoader *loader);
    bool initNonStatic(class Flint *flint, class FExec *ctx, class ClassLoader *loader);
private:
    FieldsData(const FieldsData &) = delete;
    void operator=(const FieldsData &) = delete;

    friend class Flint;
};

#endif /* __FLINT_FIELDS_DATA_H */
