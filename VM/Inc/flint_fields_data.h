
#ifndef __FLINT_FIELD_DATA_H
#define __FLINT_FIELD_DATA_H

#include "flint_java_object.h"
#include "flint_const_pool.h"
#include "flint_class_loader.h"

class FlintFieldData32 {
public:
    const FlintFieldInfo &fieldInfo;
    int32_t value;
private:
    FlintFieldData32(const FlintFieldInfo &fieldInfo);
    FlintFieldData32(const FlintFieldData32 &) = delete;
    void operator=(const FlintFieldData32 &) = delete;

    friend class FlintFieldsData;
};

class FlintFieldData64 {
public:
    const FlintFieldInfo &fieldInfo;
    int64_t value;
private:
    FlintFieldData64(const FlintFieldInfo &fieldInfo);
    FlintFieldData64(const FlintFieldData64 &) = delete;
    void operator=(const FlintFieldData64 &) = delete;

    friend class FlintFieldsData;
};

class FlintFieldObject {
public:
    const FlintFieldInfo &fieldInfo;
    FlintJavaObject *object;
private:
    FlintFieldObject(const FlintFieldInfo &fieldInfo);
    FlintFieldObject(const FlintFieldObject &) = delete;
    void operator=(const FlintFieldObject &) = delete;

    friend class FlintFieldsData;
};

class FlintFieldsData {
public:
    const uint16_t fields32Count;
    const uint16_t fields64Count;
    const uint16_t fieldsObjCount;

    FlintResult<FlintClassLoader> loadStatic(FlintClassLoader &classLoader);
    FlintResult<FlintClassLoader> loadNonStatic(Flint &flint, FlintClassLoader &classLoader);

    FlintFieldData32 *getFieldData32(const char *fieldName, uint32_t *index = 0) const;
    FlintFieldData32 *getFieldData32(FlintConstUtf8 &fieldName, uint32_t *index = 0) const;
    FlintFieldData32 *getFieldData32(FlintConstField &constField) const;
    FlintFieldData32 *getFieldData32ByIndex(int32_t index) const;

    FlintFieldData64 *getFieldData64(const char *fieldName, uint32_t *index = 0) const;
    FlintFieldData64 *getFieldData64(FlintConstUtf8 &fieldName, uint32_t *index = 0) const;
    FlintFieldData64 *getFieldData64(FlintConstField &constField) const;
    FlintFieldData64 *getFieldData64ByIndex(int32_t index) const;

    FlintFieldObject *getFieldObject(const char *fieldName, uint32_t *index = 0) const;
    FlintFieldObject *getFieldObject(FlintConstUtf8 &fieldName, uint32_t *index = 0) const;
    FlintFieldObject *getFieldObject(FlintConstField &constField) const;
    FlintFieldObject *getFieldObjectByIndex(int32_t index) const;
private:
    FlintFieldData32 *fieldsData32;
    FlintFieldData64 *fieldsData64;
    FlintFieldObject *fieldsObject;
private:
    FlintFieldsData(void);
    FlintFieldsData(const FlintFieldsData &) = delete;
    void operator=(const FlintFieldsData &) = delete;

    ~FlintFieldsData(void);

    friend class Flint;
    friend class FlintClassData;
    friend class FlintExecution;
};

#endif /* __FLINT_FIELD_DATA_H */
