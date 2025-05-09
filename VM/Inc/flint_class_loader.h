
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
    /*
    uint32_t magic;
    uint16_t minorVersion;
    uint16_t majorVersion;
    */
    uint16_t poolCount;
    uint16_t accessFlags;
    uint16_t interfacesCount;
    uint16_t fieldsCount;
    uint16_t methodsCount;
public:
    FlintConstUtf8 * const thisClass;
    FlintConstUtf8 * const superClass;
private:
    class Flint &flint;
    FlintConstPool *poolTable;
    uint16_t *interfaces;
    FlintFieldInfo *fields;
    FlintMethodInfo *methods;

    FlintClassLoader(const FlintClassLoader &) = delete;
    void operator=(const FlintClassLoader &) = delete;

    FlintError load(void *file);
    FlintError readAttributeCode(void *file, FlintMethodInfo &method);
protected:
    FlintClassLoader(class Flint &flint);

    FlintError load(const char *fileName, uint16_t length);

    ~FlintClassLoader(void);
public:
    /*
    uint32_t getMagic(void) const;
    uint16_t getMinorVersion(void) const;
    uint16_t getMajorversion(void) const;
    */

    FlintConstPool &getConstPool(uint16_t index) const;

    int32_t getConstInteger(FlintConstPool &constPool) const;
    float getConstFloat(FlintConstPool &constPool) const;
    int64_t getConstLong(FlintConstPool &constPool) const;
    double getConstDouble(FlintConstPool &constPool) const;

    FlintConstUtf8 &getConstUtf8(uint16_t poolIndex) const;
    FlintConstUtf8 &getConstUtf8(FlintConstPool &constPool) const;

    FlintConstUtf8 &getConstUtf8Class(uint16_t poolIndex) const;
    FlintConstUtf8 &getConstUtf8Class(FlintConstPool &constPool) const;
    FlintError getConstClass(FlintConstPool &constPool, FlintJavaClass *&cls);

    FlintError getConstString(FlintConstPool &constPool, FlintJavaString *&str);

    FlintConstUtf8 &getConstMethodType(FlintConstPool &constPool) const;

    FlintError getConstNameAndType(uint16_t poolIndex, FlintConstNameAndType *&constNameAndType);
    FlintError getConstField(uint16_t poolIndex, FlintConstField *&constField);
    FlintError getConstMethod(uint16_t poolIndex, FlintConstMethod *&constMethod);
    FlintError getConstInterfaceMethod(uint16_t poolIndex, FlintConstInterfaceMethod *&constInterfaceMethod);

    FlintClassAccessFlag getAccessFlag(void) const;

    uint16_t getInterfacesCount(void) const;
    FlintConstUtf8 &getInterface(uint8_t interfaceIndex) const;

    uint16_t getFieldsCount(void) const;
    FlintFieldInfo *getFieldInfo(uint8_t fieldIndex) const;
    FlintFieldInfo *getFieldInfo(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) const;
    FlintFieldInfo *getFieldInfo(FlintConstNameAndType &nameAndType) const;

    uint16_t getMethodsCount(void) const;
    FlintError getMethodInfo(uint8_t methodIndex, FlintMethodInfo *&methodInfo);
    FlintError getMethodInfo(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor, FlintMethodInfo *&methodInfo);
    FlintError getMethodInfo(FlintConstNameAndType &nameAndType, FlintMethodInfo *&methodInfo);
    FlintMethodInfo *getMethodInfoWithUnload(uint8_t methodIndex);
    FlintError getMainMethodInfo(FlintMethodInfo *&methodInfo);
    FlintError getStaticCtor(FlintMethodInfo *&methodInfo);

    bool hasStaticCtor(void);
};

#endif /* __FLINT_CLASS_LOADER_H */
