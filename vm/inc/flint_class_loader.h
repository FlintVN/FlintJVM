
#ifndef __FLINT_CLASS_LOADER_H
#define __FLINT_CLASS_LOADER_H

#include "flint_std.h"
#include "flint_dictionary.h"
#include "flint_const_pool.h"
#include "flint_fields_data.h"
#include "flint_field_info.h"
#include "flint_method_info.h"
#include "flint_system_api.h"
#include "flint_java_class.h"
#include "flint_java_string.h"
#include "flint_file_reader.h"

#define FLAG_HAS_STATIC_FIELD   0x01
#define FLAG_HAS_CLINIT         0x02
#define FLAG_STATIC_INIT        0x08

typedef enum : uint16_t {
    CLASS_PUBLIC = 0x0001,
    CLASS_FINAL = 0x0010,
    CLASS_SUPER = 0x0020,
    CLASS_INTERFACE = 0x0200,
    CLASS_ABSTRACT = 0x0400,
    CLASS_SYNTHETIC = 0x1000,
    CLASS_ANNOTATION = 0x2000,
    CLASS_ENUM = 0x4000,
    CLASS_MODULE = 0x8000
} ClassAccessFlag;

typedef enum {
    UNINITIALIZED = 0x00,
    INITIALIZING = 0x01,
    INITIALIZED = 0x02,
} StaticInitStatus;

class ClassLoader : public DictNode {
private:
    uint8_t loaderFlags;
    /*
    uint32_t magic;
    uint16_t minorVersion;
    uint16_t majorVersion;
    */
    uint16_t poolCount;
    uint16_t accessFlags;
    uint16_t thisClass;
    uint16_t superClass;
    uint16_t interfacesCount;
    uint16_t fieldsCount;
    uint16_t methodsCount;
    uint16_t nestHost;
    uint16_t nestMembersCount;

    uint32_t hash;
public:
    uint32_t monitorOwnId;
    uint32_t monitorCount;
private:
    class Flint * const flint;
    ConstPool *poolTable;
    JClass **interfaces;
    FieldInfo *fields;
    MethodInfo *methods;
    uint16_t *nestMembers;
    FieldsData *staticFields;
    const char *filePath;
public:
    uint32_t getHashKey(void) const override;
    int32_t compareKey(const char *key, uint16_t length) const override;
    int32_t compareKey(DictNode *other) const override;

    inline __attribute__((always_inline)) ConstPoolTag getConstPoolTag(uint16_t poolIndex) const {
        return (ConstPoolTag)(poolTable[poolIndex - 1].tag & 0x7F);
    }

    inline __attribute__((always_inline)) int32_t getConstInteger(uint16_t poolIndex) const {
        return (int32_t)poolTable[poolIndex - 1].value;
    }

    inline __attribute__((always_inline)) float getConstFloat(uint16_t poolIndex) const {
        return *(float *)&poolTable[poolIndex - 1].value;
    }

    inline __attribute__((always_inline)) int64_t getConstLong(uint16_t poolIndex) const {
        return ((uint64_t)poolTable[poolIndex].value << 32) | poolTable[poolIndex - 1].value;
    }

    inline __attribute__((always_inline)) double getConstDouble(uint16_t poolIndex) const {
        uint64_t ret = ((uint64_t)poolTable[poolIndex].value << 32) | poolTable[poolIndex - 1].value;
        return *(double *)&ret;
    }

    const char *getConstUtf8(uint16_t poolIndex) const;
    const char *getConstClassName(uint16_t poolIndex) const;

    ConstNameAndType *getConstNameAndType(FExec *ctx, uint16_t poolIndex);
    ConstField *getConstField(FExec *ctx, uint16_t poolIndex);
    ConstMethod *getConstMethod(FExec *ctx, uint16_t poolIndex);
    ConstInterfaceMethod *getConstInterfaceMethod(FExec *ctx, uint16_t poolIndex);

    JString *getConstString(FExec *ctx, uint16_t poolIndex);
    JClass *getConstClass(FExec *ctx, uint16_t poolIndex);

    ClassAccessFlag getAccessFlag(void) const;

    const char *getFilePath(void) const;
    const char *getName(void) const;
    const char *getSuperClassName(void) const;

    JClass *getThisClass(FExec *ctx);
    JClass *getSuperClass(FExec *ctx);

    uint16_t getInterfacesCount(void) const;
    JClass *getInterface(FExec *ctx, uint16_t interfaceIndex);
    const char *getInterfaceName(uint16_t interfaceIndex) const;

    uint16_t getFieldsCount(void) const;
    FieldInfo *getFieldInfo(uint16_t fieldIndex) const;

    uint16_t getMethodsCount(void) const;
    MethodInfo *getMethodInfo(FExec *ctx, uint16_t methodIndex);
    MethodInfo *getMethodInfo(FExec *ctx, ConstNameAndType *nameAndType);
    MethodInfo *getMethodInfo(FExec *ctx, const char *name, const char *desc);
    MethodInfo *getMainMethodInfo(FExec *ctx);
    MethodInfo *getStaticCtor(FExec *ctx);

    bool hasStaticField(void) const;
    bool hasStaticCtor(void) const;

    JClass *getNestHost(FExec *ctx);

    uint16_t getNestMembersCount(void) const;
    JClass *getNestMember(FExec *ctx, uint16_t index);

    uint16_t hasStaticObjField(void) const;
    FieldValue *getStaticField(FExec *ctx, ConstField *field) const;
    FieldValue *getStaticField(FExec *ctx, const char *name) const;
    FieldValue *getStaticFieldByIndex(uint32_t index) const;

    inline __attribute__((always_inline)) StaticInitStatus getStaticInitStatus(void) const {
        if (loaderFlags & FLAG_STATIC_INIT)
            return INITIALIZED;
        return (staticFields == NULL) ? UNINITIALIZED : INITIALIZING;
    }

    void staticInitialized(void);
    bool initStaticFields(FExec *ctx);
    void clearStaticFields(void);
private:
    ClassLoader(Flint *flint);
    ClassLoader(const ClassLoader &) = delete;
    void operator=(const ClassLoader &) = delete;

    bool load(FileReader *reader);
    CodeAttribute *readAttributeCode(FileReader *reader);
    Exceptions *readAttributeExceptions(FileReader *reader);
public:
    static ClassLoader *load(Flint *flint, FExec *ctx, const char *clsName, uint16_t length = 0xFFFF);

    ~ClassLoader(void);
};

#endif /* __FLINT_CLASS_LOADER_H */
