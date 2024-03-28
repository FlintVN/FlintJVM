
#ifndef __MJVM_CLASS_LOADER_H
#define __MJVM_CLASS_LOADER_H

#include "mjvm_class_file.h"
#include "mjvm_const_pool.h"
#include "mjvm_field_info.h"
#include "mjvm_method_info.h"

class ClassLoader {
private:
    const char *fileName;

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

    void load(ClassFile &file);
    AttributeInfo &readAttribute(ClassFile &file);
    AttributeInfo &readAttributeCode(ClassFile &file);
    AttributeInfo &readAttributeLineNumberTable(ClassFile &file);
    AttributeInfo &readAttributeLocalVariableTable(ClassFile &file);
public:
    ClassLoader(const char *fileName);
    const uint32_t getMagic(void) const;
    const uint16_t getMinorVersion(void) const;
    const uint16_t getMajorversion(void) const;

    const int32_t getConstInteger(uint16_t poolIndex) const;
    const float getConstFloat(uint16_t poolIndex) const;
    const int64_t getConstLong(uint16_t poolIndex) const;
    const double getConstDouble(uint16_t poolIndex) const;

    const ConstUtf8 &getConstUtf8(uint16_t poolIndex) const;
    const ConstUtf8 &getConstClass(uint16_t poolIndex) const;
    const ConstUtf8 &getConstString(uint16_t poolIndex) const;
    const ConstUtf8 &getConstMethodType(uint16_t poolIndex) const;

    const ConstNameAndType &getConstNameAndType(uint16_t poolIndex) const;
    const ConstField &getConstField(uint16_t poolIndex) const;
    const ConstMethod &getConstMethod(uint16_t poolIndex) const;
    const ConstInterfaceMethod &getConstInterfaceMethod(uint16_t poolIndex) const;

    const ClassAccessFlag getAccessFlag(void) const;

    const ConstUtf8 &getThisClass(void) const;
    const ConstUtf8 &getSupperClass(void) const;

    const ConstUtf8 &getInterface(uint8_t interfaceIndex) const;

    const FieldInfo &getFieldInfo(uint8_t fieldIndex) const;
    const FieldInfo &getFieldInfo(const ConstNameAndType &field) const;
    const MethodInfo &getMethodInfo(uint8_t methodIndex) const;
    const MethodInfo &getMethodInfo(const ConstNameAndType &method) const;
    const MethodInfo &getMainMethodInfo(void) const;

    ~ClassLoader(void);
};

#endif /* __MJVM_CLASS_LOADER_H */
