
#ifndef __MJVM_CLASS_LOADER_H
#define __MJVM_CLASS_LOADER_H

#include "mjvm_const_pool.h"
#include "mjvm_field_info.h"
#include "mjvm_method_info.h"
#include "mjvm_class.h"
#include "mjvm_string.h"

class MjvmExecution;

class MjvmClassLoader {
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
    MjvmConstPool *poolTable;
    uint16_t *interfaces;
    MjvmFieldInfo *fields;
    MjvmMethodInfo *methods;
    MjvmAttribute *attributes;

    MjvmClassLoader(const MjvmClassLoader &) = delete;
    void operator=(const MjvmClassLoader &) = delete;

    void addAttribute(MjvmAttribute *attribute);

    void readFile(void *file);
    MjvmAttribute *readAttribute(void *file, bool isDummy = false);
    MjvmAttribute *readAttributeCode(void *file);
    MjvmAttribute *readAttributeBootstrapMethods(void *file);
protected:
    MjvmClassLoader(const char *fileName);
    MjvmClassLoader(const char *fileName, uint16_t length);
    MjvmClassLoader(const MjvmConstUtf8 &fileName);

    ~MjvmClassLoader(void);
public:
    uint32_t getMagic(void) const;
    uint16_t getMinorVersion(void) const;
    uint16_t getMajorversion(void) const;

    MjvmConstPool &getConstPool(uint16_t index) const;

    int32_t getConstInteger(uint16_t poolIndex) const;
    int32_t getConstInteger(MjvmConstPool &constPool) const;
    float getConstFloat(uint16_t poolIndex) const;
    float getConstFloat(MjvmConstPool &constPool) const;
    int64_t getConstLong(uint16_t poolIndex) const;
    int64_t getConstLong(MjvmConstPool &constPool) const;
    double getConstDouble(uint16_t poolIndex) const;
    double getConstDouble(MjvmConstPool &constPool) const;

    MjvmConstUtf8 &getConstUtf8(uint16_t poolIndex) const;
    MjvmConstUtf8 &getConstUtf8(MjvmConstPool &constPool) const;

    MjvmConstUtf8 &getConstUtf8Class(uint16_t poolIndex) const;
    MjvmConstUtf8 &getConstUtf8Class(MjvmConstPool &constPool) const;
    MjvmClass &getConstClass(MjvmExecution &execution, uint16_t poolIndex);
    MjvmClass &getConstClass(MjvmExecution &execution, MjvmConstPool &constPool);

    MjvmString &getConstString(MjvmExecution &execution, uint16_t poolIndex);
    MjvmString &getConstString(MjvmExecution &execution, MjvmConstPool &constPool);

    MjvmConstUtf8 &getConstMethodType(uint16_t poolIndex) const;
    MjvmConstUtf8 &getConstMethodType(MjvmConstPool &constPool) const;

    MjvmConstNameAndType &getConstNameAndType(uint16_t poolIndex);
    MjvmConstNameAndType &getConstNameAndType(MjvmConstPool &constPool);
    MjvmConstField &getConstField(uint16_t poolIndex);
    MjvmConstField &getConstField(MjvmConstPool &constPool);
    MjvmConstMethod &getConstMethod(uint16_t poolIndex);
    MjvmConstMethod &getConstMethod(MjvmConstPool &constPool);
    MjvmConstInterfaceMethod &getConstInterfaceMethod(uint16_t poolIndex);
    MjvmConstInterfaceMethod &getConstInterfaceMethod(MjvmConstPool &constPool);

    MjvmClassAccessFlag getAccessFlag(void) const;

    MjvmConstUtf8 &getThisClass(void) const;
    MjvmConstUtf8 &getSuperClass(void) const;

    uint16_t getInterfacesCount(void) const;
    MjvmConstUtf8 &getInterface(uint8_t interfaceIndex) const;

    uint16_t getFieldsCount(void) const;
    MjvmFieldInfo &getFieldInfo(uint8_t fieldIndex) const;
    MjvmFieldInfo &getFieldInfo(MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) const;
    MjvmFieldInfo &getFieldInfo(MjvmConstNameAndType &nameAndType) const;

    uint16_t getMethodsCount(void) const;
    MjvmMethodInfo &getMethodInfo(uint8_t methodIndex) const;
    MjvmMethodInfo &getMethodInfo(MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) const;
    MjvmMethodInfo &getMethodInfo(MjvmConstNameAndType &nameAndType) const;
    MjvmMethodInfo &getMainMethodInfo(void) const;
    MjvmMethodInfo &getStaticConstructor(void) const;
};

#endif /* __MJVM_CLASS_LOADER_H */
