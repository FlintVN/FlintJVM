
#ifndef __MJVM_FIELD_DATA_H
#define __MJVM_FIELD_DATA_H

#include "mjvm_object.h"
#include "mjvm_class_loader.h"

class MjvmExecution;

class FieldData32 {
public:
    const MjvmFieldInfo &fieldInfo;
    int32_t value;
private:
    FieldData32(const MjvmFieldInfo &fieldInfo);
    FieldData32(const FieldData32 &) = delete;
    void operator=(const FieldData32 &) = delete;

    friend class MjvmFieldsData;
};

class FieldData64 {
public:
    const MjvmFieldInfo &fieldInfo;
    int64_t value;
private:
    FieldData64(const MjvmFieldInfo &fieldInfo);
    FieldData64(const FieldData64 &) = delete;
    void operator=(const FieldData64 &) = delete;

    friend class MjvmFieldsData;
};

class FieldObject {
public:
    const MjvmFieldInfo &fieldInfo;
    MjvmObject *object;
private:
    FieldObject(const MjvmFieldInfo &fieldInfo);
    FieldObject(const FieldObject &) = delete;
    void operator=(const FieldObject &) = delete;

    friend class MjvmFieldsData;
};

class MjvmFieldsData {
public:
    const uint16_t fields32Count;
    const uint16_t fields64Count;
    const uint16_t fieldsObjCount;

    FieldData32 &getFieldData32(const MjvmConstNameAndType &fieldName) const;
    FieldData64 &getFieldData64(const MjvmConstNameAndType &fieldName) const;
    FieldObject &getFieldObject(const MjvmConstNameAndType &fieldName) const;
private:
    FieldData32 *fieldsData32;
    FieldData64 *fieldsData64;
    FieldObject *fieldsObject;
private:
    MjvmFieldsData(MjvmExecution &execution, const MjvmClassLoader &classLoader, bool isStatic);
    MjvmFieldsData(const MjvmFieldsData &) = delete;
    void operator=(const MjvmFieldsData &) = delete;

    void loadStatic(const MjvmClassLoader &classLoader);
    void loadNonStatic(MjvmExecution &execution, const MjvmClassLoader &classLoader);

    ~MjvmFieldsData(void);

    friend class MjvmExecution;
    friend class ClassData;
};

class ClassData : public MjvmClassLoader {
private:
    ClassData *next;
public:
    uint32_t ownId;
    uint32_t monitorCount : 31;
    uint32_t isInitializing : 1;
    MjvmFieldsData *staticFiledsData;

    ClassData(const char *fileName);
    ClassData(const char *fileName, uint16_t length);
    ClassData(const MjvmConstUtf8 &fileName);

    ClassData(const ClassData &) = delete;
    void operator=(const ClassData &) = delete;

    ~ClassData(void);

    friend class MjvmExecution;
};

#endif /* __MJVM_FIELD_DATA_H */
