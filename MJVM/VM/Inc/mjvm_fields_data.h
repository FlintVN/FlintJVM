
#ifndef __MJVM_FIELD_DATA_H
#define __MJVM_FIELD_DATA_H

#include "mjvm_object.h"
#include "mjvm_class_loader.h"

class Execution;

class FieldData8 {
public:
    const FieldInfo &fieldInfo;
    int8_t value;
private:
    FieldData8(const FieldInfo &fieldInfo);
    FieldData8(const FieldData8 &) = delete;
    void operator=(const FieldData8 &) = delete;

    friend class FieldsData;
};

class FieldData16 {
public:
    const FieldInfo &fieldInfo;
    int16_t value;
private:
    FieldData16(const FieldInfo &fieldInfo);
    FieldData16(const FieldData16 &) = delete;
    void operator=(const FieldData16 &) = delete;

    friend class FieldsData;
};

class FieldData32 {
public:
    const FieldInfo &fieldInfo;
    int32_t value;
private:
    FieldData32(const FieldInfo &fieldInfo);
    FieldData32(const FieldData32 &) = delete;
    void operator=(const FieldData32 &) = delete;

    friend class FieldsData;
};

class FieldData64 {
public:
    const FieldInfo &fieldInfo;
    int64_t value;
private:
    FieldData64(const FieldInfo &fieldInfo);
    FieldData64(const FieldData64 &) = delete;
    void operator=(const FieldData64 &) = delete;

    friend class FieldsData;
};

class FieldObject {
public:
    const FieldInfo &fieldInfo;
    MjvmObject *object;
private:
    FieldObject(const FieldInfo &fieldInfo);
    FieldObject(const FieldObject &) = delete;
    void operator=(const FieldObject &) = delete;

    friend class FieldsData;
};

class FieldsData {
public:
    const uint16_t fields8Count;
    const uint16_t fields16Count;
    const uint16_t fields32Count;
    const uint16_t fields64Count;
    const uint16_t fieldsObjCount;

    FieldData8 &getFieldData8(const ConstNameAndType &fieldName) const;
    FieldData16 &getFieldData16(const ConstNameAndType &fieldName) const;
    FieldData32 &getFieldData32(const ConstNameAndType &fieldName) const;
    FieldData64 &getFieldData64(const ConstNameAndType &fieldName) const;
    FieldObject &getFieldObject(const ConstNameAndType &fieldName) const;
private:
    FieldData8 *fieldsData8;
    FieldData16 *fieldsData16;
    FieldData32 *fieldsData32;
    FieldData64 *fieldsData64;
    FieldObject *fieldsObject;
private:
    FieldsData(Execution &execution, const ClassLoader &classLoader, bool isStatic);
    FieldsData(const FieldsData &) = delete;
    void operator=(const FieldsData &) = delete;

    void loadStatic(const ClassLoader &classLoader);
    void loadNonStatic(Execution &execution, const ClassLoader &classLoader);

    ~FieldsData(void);

    friend class Execution;
    friend class ClassData;
};

class ClassData {
public:
    const ClassLoader &classLoader;
    FieldsData *filedsData;
protected:
    ClassData(const ClassLoader &classLoader, FieldsData *filedsData);

    ~ClassData(void);
private:
    ClassData(const ClassData &) = delete;
    void operator=(const ClassData &) = delete;

    friend class Execution;
};

class ClassDataNode : public ClassData {
private:
    uint32_t ownId;
    uint32_t monitorCount;
    ClassDataNode *next;

    ClassDataNode(const ClassLoader &classLoader, FieldsData *filedsData);
    ClassDataNode(const ClassDataNode &) = delete;
    void operator=(const ClassDataNode &) = delete;

    friend class Execution;
};

#endif /* __MJVM_FIELD_DATA_H */
