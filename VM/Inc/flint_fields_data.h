
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

    FlintFieldData32 &getFieldData32(const char *fieldName) const;
    FlintFieldData32 &getFieldData32(const FlintConstUtf8 &fieldName) const;
    FlintFieldData32 &getFieldData32(FlintConstField &constField) const;
    FlintFieldData32 &getFieldData32ByIndex(int32_t index) const;

    FlintFieldData64 &getFieldData64(const char *fieldName) const;
    FlintFieldData64 &getFieldData64(const FlintConstUtf8 &fieldName) const;
    FlintFieldData64 &getFieldData64(FlintConstField &constField) const;
    FlintFieldData64 &getFieldData64ByIndex(int32_t index) const;

    FlintFieldObject &getFieldObject(const char *fieldName) const;
    FlintFieldObject &getFieldObject(const FlintConstUtf8 &fieldName) const;
    FlintFieldObject &getFieldObject(FlintConstField &constField) const;
    FlintFieldObject &getFieldObjectByIndex(int32_t index) const;

private:
    FlintFieldData32 *fieldsData32;
    FlintFieldData64 *fieldsData64;
    FlintFieldObject *fieldsObject;
private:
    FlintFieldsData(Flint &flint, const FlintClassLoader &classLoader, bool isStatic);
    FlintFieldsData(const FlintFieldsData &) = delete;
    void operator=(const FlintFieldsData &) = delete;

    void loadStatic(const FlintClassLoader &classLoader);
    void loadNonStatic(Flint &flint, const FlintClassLoader &classLoader);

    ~FlintFieldsData(void);

    friend class Flint;
    friend class ClassData;
    friend class FlintExecution;
};

class ClassData : public FlintClassLoader {
private:
    ClassData *next;
public:
    uint32_t ownId;
    uint32_t monitorCount : 31;
    uint32_t isInitializing : 1;
    FlintFieldsData *staticFieldsData;
private:
    ClassData(class Flint &flint, const char *fileName);
    ClassData(class Flint &flint, const char *fileName, uint16_t length);
    ClassData(class Flint &flint, const FlintConstUtf8 &fileName);

    ClassData(const ClassData &) = delete;
    void operator=(const ClassData &) = delete;

    void clearStaticFields(void);

    ~ClassData(void);

    friend class Flint;
};

#endif /* __FLINT_FIELD_DATA_H */
