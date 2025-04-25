
#ifndef __FLINT_CLASS_LOADER_H
#define __FLINT_CLASS_LOADER_H

#include "flint_const_pool.h"
#include "flint_field_info.h"
#include "flint_method_info.h"
#include "flint_java_class.h"
#include "flint_java_string.h"

class FlintClassLoader {
private:
    uint8_t staticCtorInfo;
    uint32_t magic;
    uint16_t minorVersion;
    uint16_t majorVersion;
    uint16_t poolCount;
    uint16_t accessFlags;
    uint16_t interfacesCount;
    uint16_t fieldsCount;
    uint16_t methodsCount;
    FlintConstUtf8 *thisClass;
    FlintConstUtf8 *superClass;
    FlintConstPool *poolTable;
    uint16_t *interfaces;
    FlintFieldInfo *fields;
    FlintMethodInfo *methods;

    FlintClassLoader(const FlintClassLoader &) = delete;
    void operator=(const FlintClassLoader &) = delete;

    void readFile(class Flint &flint, void *file);
    void readAttributeCode(void *file, FlintMethodInfo &method);
protected:
    FlintClassLoader(class Flint &flint, const char *fileName, uint16_t length);

    ~FlintClassLoader(void);
public:
    uint32_t getMagic(void) const;
    uint16_t getMinorVersion(void) const;
    uint16_t getMajorversion(void) const;

    FlintConstPool &getConstPool(uint16_t index) const;

    int32_t getConstInteger(uint16_t poolIndex) const;
    int32_t getConstInteger(FlintConstPool &constPool) const;
    float getConstFloat(uint16_t poolIndex) const;
    float getConstFloat(FlintConstPool &constPool) const;
    int64_t getConstLong(uint16_t poolIndex) const;
    int64_t getConstLong(FlintConstPool &constPool) const;
    double getConstDouble(uint16_t poolIndex) const;
    double getConstDouble(FlintConstPool &constPool) const;

    FlintConstUtf8 &getConstUtf8(uint16_t poolIndex) const;
    FlintConstUtf8 &getConstUtf8(FlintConstPool &constPool) const;

    FlintConstUtf8 &getConstUtf8Class(uint16_t poolIndex) const;
    FlintConstUtf8 &getConstUtf8Class(FlintConstPool &constPool) const;
    FlintJavaClass &getConstClass(class Flint &flint, uint16_t poolIndex);
    FlintJavaClass &getConstClass(class Flint &flint, FlintConstPool &constPool);

    FlintJavaString &getConstString(class Flint &flint, uint16_t poolIndex);
    FlintJavaString &getConstString(class Flint &flint, FlintConstPool &constPool);

    FlintConstUtf8 &getConstMethodType(uint16_t poolIndex) const;
    FlintConstUtf8 &getConstMethodType(FlintConstPool &constPool) const;

    FlintConstNameAndType &getConstNameAndType(uint16_t poolIndex);
    FlintConstNameAndType &getConstNameAndType(FlintConstPool &constPool);
    FlintConstField &getConstField(uint16_t poolIndex);
    FlintConstField &getConstField(FlintConstPool &constPool);
    FlintConstMethod &getConstMethod(uint16_t poolIndex);
    FlintConstMethod &getConstMethod(FlintConstPool &constPool);
    FlintConstInterfaceMethod &getConstInterfaceMethod(uint16_t poolIndex);
    FlintConstInterfaceMethod &getConstInterfaceMethod(FlintConstPool &constPool);

    FlintClassAccessFlag getAccessFlag(void) const;

    FlintConstUtf8 &getThisClass(void) const;
    FlintConstUtf8 &getSuperClass(void) const;

    uint16_t getInterfacesCount(void) const;
    FlintConstUtf8 &getInterface(uint8_t interfaceIndex) const;

    uint16_t getFieldsCount(void) const;
    FlintFieldInfo &getFieldInfo(uint8_t fieldIndex) const;
    FlintFieldInfo &getFieldInfo(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) const;
    FlintFieldInfo &getFieldInfo(FlintConstNameAndType &nameAndType) const;

    uint16_t getMethodsCount(void) const;
    FlintMethodInfo *getMethodInfo(uint8_t methodIndex);
    FlintMethodInfo *getMethodInfo(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor);
    FlintMethodInfo *getMethodInfo(FlintConstNameAndType &nameAndType);
    FlintMethodInfo *getMethodInfoWithUnload(uint8_t methodIndex);
    FlintMethodInfo *getMethodInfoWithUnload(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor);
    FlintMethodInfo *getMainMethodInfo(void);
    FlintMethodInfo *getStaticCtor(void);

    bool hasStaticCtor(void);
};

#endif /* __FLINT_CLASS_LOADER_H */
