
#ifndef __MJVM_CLASS_LOADER_H
#define __MJVM_CLASS_LOADER_H

#include "mjvm_class_file.h"
#include "mjvm_const_pool.h"
#include "mjvm_field_info.h"
#include "mjvm_method_info.h"

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
    AttributeInfo **attributes;

    ClassLoader(const ClassLoader &) = delete;
    void operator=(const ClassLoader &) = delete;

    void readFile(ClassFile &file);
    AttributeInfo &readAttribute(ClassFile &file);
    AttributeInfo &readAttributeCode(ClassFile &file);
    AttributeInfo &readAttributeLineNumberTable(ClassFile &file);
    AttributeInfo &readAttributeLocalVariableTable(ClassFile &file);
    AttributeInfo &readAttributeBootstrapMethods(ClassFile &file);
protected:
    ClassLoader(const char *fileName);
    ClassLoader(const char *fileName, uint16_t length);
    ClassLoader(const ConstUtf8 &fileName);

    ~ClassLoader(void);
public:
    const uint32_t getMagic(void) const;
    const uint16_t getMinorVersion(void) const;
    const uint16_t getMajorversion(void) const;

    const ConstPool &getConstPool(uint16_t index) const;

    const int32_t getConstInteger(uint16_t poolIndex) const;
    const int32_t getConstInteger(const ConstPool &constPool) const;
    const float getConstFloat(uint16_t poolIndex) const;
    const float getConstFloat(const ConstPool &constPool) const;
    const int64_t getConstLong(uint16_t poolIndex) const;
    const int64_t getConstLong(const ConstPool &constPool) const;
    const double getConstDouble(uint16_t poolIndex) const;
    const double getConstDouble(const ConstPool &constPool) const;

    const ConstUtf8 &getConstUtf8(uint16_t poolIndex) const;
    const ConstUtf8 &getConstUtf8(const ConstPool &constPool) const;
    const ConstUtf8 &getConstClass(uint16_t poolIndex) const;
    const ConstUtf8 &getConstClass(const ConstPool &constPool) const;
    const ConstUtf8 &getConstString(uint16_t poolIndex) const;
    const ConstUtf8 &getConstString(const ConstPool &constPool) const;
    const ConstUtf8 &getConstMethodType(uint16_t poolIndex) const;
    const ConstUtf8 &getConstMethodType(const ConstPool &constPool) const;

    const ConstNameAndType &getConstNameAndType(uint16_t poolIndex) const;
    const ConstNameAndType &getConstNameAndType(const ConstPool &constPool) const;
    const ConstField &getConstField(uint16_t poolIndex) const;
    const ConstField &getConstField(const ConstPool &constPool) const;
    const ConstMethod &getConstMethod(uint16_t poolIndex) const;
    const ConstMethod &getConstMethod(const ConstPool &constPool) const;
    const ConstInterfaceMethod &getConstInterfaceMethod(uint16_t poolIndex) const;
    const ConstInterfaceMethod &getConstInterfaceMethod(const ConstPool &constPool) const;

    const ClassAccessFlag getAccessFlag(void) const;

    const ConstUtf8 &getThisClass(void) const;
    const ConstUtf8 &getSuperClass(void) const;

    const uint16_t getInterfacesCount(void) const;
    const ConstUtf8 &getInterface(uint8_t interfaceIndex) const;

    const uint16_t getFieldsCount(void) const;
    const FieldInfo &getFieldInfo(uint8_t fieldIndex) const;
    const FieldInfo &getFieldInfo(const ConstNameAndType &fieldName) const;

    const uint16_t getMethodsCount(void) const;
    const MethodInfo &getMethodInfo(uint8_t methodIndex) const;
    const MethodInfo &getMethodInfo(const ConstNameAndType &methodName) const;
    const MethodInfo &getMainMethodInfo(void) const;
    const MethodInfo &getStaticConstructor(void) const;
};

#endif /* __MJVM_CLASS_LOADER_H */
