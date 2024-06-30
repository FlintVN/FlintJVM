
#ifndef __MJVM_CLASS_LOADER_H
#define __MJVM_CLASS_LOADER_H

#include "mjvm_const_pool.h"
#include "mjvm_field_info.h"
#include "mjvm_method_info.h"
#include "mjvm_class.h"
#include "mjvm_string.h"

class Execution;

class ClassLoader {
private:
    uint32_t magic;
    uint16_t minorVersion;
    uint16_t majorVersion;
    uint16_t poolCount;
    uint16_t accessFlags;
    uint16_t thisClass;
    uint16_t superClass;
    uint16_t interfacesCount;
    uint16_t fieldsCount;
    uint16_t methodsCount;
    uint16_t attributesCount;
    ConstPool *poolTable;
    uint16_t *interfaces;
    FieldInfo *fields;
    MethodInfo *methods;
    AttributeInfo *attributes;

    ClassLoader(const ClassLoader &) = delete;
    void operator=(const ClassLoader &) = delete;

    void addAttribute(AttributeInfo *attribute);

    void readFile(void *file);
    AttributeInfo *readAttribute(void *file);
    AttributeInfo *readAttributeCode(void *file);
    AttributeInfo *readAttributeBootstrapMethods(void *file);
protected:
    ClassLoader(const char *fileName);
    ClassLoader(const char *fileName, uint16_t length);
    ClassLoader(const ConstUtf8 &fileName);

    ~ClassLoader(void);
public:
    uint32_t getMagic(void) const;
    uint16_t getMinorVersion(void) const;
    uint16_t getMajorversion(void) const;

    ConstPool &getConstPool(uint16_t index) const;

    int32_t getConstInteger(uint16_t poolIndex) const;
    int32_t getConstInteger(ConstPool &constPool) const;
    float getConstFloat(uint16_t poolIndex) const;
    float getConstFloat(ConstPool &constPool) const;
    int64_t getConstLong(uint16_t poolIndex) const;
    int64_t getConstLong(ConstPool &constPool) const;
    double getConstDouble(uint16_t poolIndex) const;
    double getConstDouble(ConstPool &constPool) const;

    ConstUtf8 &getConstUtf8(uint16_t poolIndex) const;
    ConstUtf8 &getConstUtf8(ConstPool &constPool) const;

    ConstUtf8 &getConstUtf8Class(uint16_t poolIndex) const;
    ConstUtf8 &getConstUtf8Class(ConstPool &constPool) const;
    MjvmClass &getConstClass(Execution &execution, uint16_t poolIndex);
    MjvmClass &getConstClass(Execution &execution, ConstPool &constPool);

    MjvmString &getConstString(Execution &execution, uint16_t poolIndex);
    MjvmString &getConstString(Execution &execution, ConstPool &constPool);

    ConstUtf8 &getConstMethodType(uint16_t poolIndex) const;
    ConstUtf8 &getConstMethodType(ConstPool &constPool) const;

    ConstNameAndType &getConstNameAndType(uint16_t poolIndex);
    ConstNameAndType &getConstNameAndType(ConstPool &constPool);
    ConstField &getConstField(uint16_t poolIndex);
    ConstField &getConstField(ConstPool &constPool);
    ConstMethod &getConstMethod(uint16_t poolIndex);
    ConstMethod &getConstMethod(ConstPool &constPool);
    ConstInterfaceMethod &getConstInterfaceMethod(uint16_t poolIndex);
    ConstInterfaceMethod &getConstInterfaceMethod(ConstPool &constPool);

    ClassAccessFlag getAccessFlag(void) const;

    ConstUtf8 &getThisClass(void) const;
    ConstUtf8 &getSuperClass(void) const;

    uint16_t getInterfacesCount(void) const;
    ConstUtf8 &getInterface(uint8_t interfaceIndex) const;

    uint16_t getFieldsCount(void) const;
    FieldInfo &getFieldInfo(uint8_t fieldIndex) const;
    FieldInfo &getFieldInfo(ConstUtf8 &name, ConstUtf8 &descriptor) const;
    FieldInfo &getFieldInfo(ConstNameAndType &nameAndType) const;

    uint16_t getMethodsCount(void) const;
    MethodInfo &getMethodInfo(uint8_t methodIndex) const;
    MethodInfo &getMethodInfo(ConstUtf8 &name, ConstUtf8 &descriptor) const;
    MethodInfo &getMethodInfo(ConstNameAndType &nameAndType) const;
    MethodInfo &getMainMethodInfo(void) const;
    MethodInfo &getStaticConstructor(void) const;
};

#endif /* __MJVM_CLASS_LOADER_H */
