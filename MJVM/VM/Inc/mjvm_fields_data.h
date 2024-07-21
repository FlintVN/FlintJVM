
#ifndef __MJVM_FIELD_DATA_H
#define __MJVM_FIELD_DATA_H

#include "mjvm_object.h"
#include "mjvm_class_loader.h"

class Mjvm;

class MjvmFieldData32 {
public:
    const MjvmFieldInfo &fieldInfo;
    int32_t value;
private:
    MjvmFieldData32(const MjvmFieldInfo &fieldInfo);
    MjvmFieldData32(const MjvmFieldData32 &) = delete;
    void operator=(const MjvmFieldData32 &) = delete;

    friend class MjvmFieldsData;
};

class MjvmFieldData64 {
public:
    const MjvmFieldInfo &fieldInfo;
    int64_t value;
private:
    MjvmFieldData64(const MjvmFieldInfo &fieldInfo);
    MjvmFieldData64(const MjvmFieldData64 &) = delete;
    void operator=(const MjvmFieldData64 &) = delete;

    friend class MjvmFieldsData;
};

class MjvmFieldObject {
public:
    const MjvmFieldInfo &fieldInfo;
    MjvmObject *object;
private:
    MjvmFieldObject(const MjvmFieldInfo &fieldInfo);
    MjvmFieldObject(const MjvmFieldObject &) = delete;
    void operator=(const MjvmFieldObject &) = delete;

    friend class MjvmFieldsData;
};

class MjvmFieldsData {
public:
    const uint16_t fields32Count;
    const uint16_t fields64Count;
    const uint16_t fieldsObjCount;

    MjvmFieldData32 &getFieldData32(const MjvmConstUtf8 &fieldName) const;
    MjvmFieldData32 &getFieldData32(const MjvmConstNameAndType &fieldNameAndType) const;
    MjvmFieldData64 &getFieldData64(const MjvmConstUtf8 &fieldName) const;
    MjvmFieldData64 &getFieldData64(const MjvmConstNameAndType &fieldNameAndType) const;
    MjvmFieldObject &getFieldObject(const MjvmConstUtf8 &fieldName) const;
    MjvmFieldObject &getFieldObject(const MjvmConstNameAndType &fieldNameAndType) const;
private:
    MjvmFieldData32 *fieldsData32;
    MjvmFieldData64 *fieldsData64;
    MjvmFieldObject *fieldsObject;
private:
    MjvmFieldsData(Mjvm &mjvm, const MjvmClassLoader &classLoader, bool isStatic);
    MjvmFieldsData(const MjvmFieldsData &) = delete;
    void operator=(const MjvmFieldsData &) = delete;

    void loadStatic(const MjvmClassLoader &classLoader);
    void loadNonStatic(Mjvm &mjvm, const MjvmClassLoader &classLoader);

    ~MjvmFieldsData(void);

    friend class Mjvm;
    friend class ClassData;
    friend class MjvmExecution;
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

    friend class Mjvm;
};

#endif /* __MJVM_FIELD_DATA_H */
