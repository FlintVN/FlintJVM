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
    ConstPool *poolTable;
    uint16_t *interfaces;
    FieldInfo *fields;
    MethodInfo *methods;
    uint16_t *nestMembers;
    FieldsData *staticFields;
public:
    uint32_t getHashKey(void) const override;
    int32_t compareKey(const char *key, uint16_t length) const override;
    int32_t compareKey(DictNode *other) const override;

    ConstPoolTag getConstPoolTag(uint16_t poolIndex) const;
    int32_t getConstInteger(uint16_t poolIndex) const;
    float getConstFloat(uint16_t poolIndex) const;
    int64_t getConstLong(uint16_t poolIndex) const;
    double getConstDouble(uint16_t poolIndex) const;
    const char *getConstUtf8(uint16_t poolIndex) const;
    const char *getConstClassName(uint16_t poolIndex) const;

    ConstNameAndType *getConstNameAndType(FExec *ctx, uint16_t poolIndex);
    ConstField *getConstField(FExec *ctx, uint16_t poolIndex);
    ConstMethod *getConstMethod(FExec *ctx, uint16_t poolIndex);
    ConstInterfaceMethod *getConstInterfaceMethod(FExec *ctx, uint16_t poolIndex);

    JString *getConstString(FExec *ctx, uint16_t poolIndex);
    JClass *getConstClass(FExec *ctx, uint16_t poolIndex);

    ClassAccessFlag getAccessFlag(void) const;

    const char *getName(void) const;
    const char *getSuperClassName(void) const;

    JClass *getThisClass(FExec *ctx);
    JClass *getSuperClass(FExec *ctx);

    uint16_t getInterfacesCount(void) const;
    const char *getInterface(uint16_t interfaceIndex) const;

    uint16_t getFieldsCount(void) const;
    FieldInfo *getFieldInfo(uint16_t fieldIndex) const;

    uint16_t getMethodsCount(void) const;
    MethodInfo *getMethodInfo(FExec *ctx, uint8_t methodIndex);
    MethodInfo *getMethodInfo(FExec *ctx, ConstNameAndType *nameAndType);
    MethodInfo *getMethodInfo(FExec *ctx, const char *name, const char *desc);
    MethodInfo *getMainMethodInfo(FExec *ctx);
    MethodInfo *getStaticCtor(FExec *ctx);

    bool hasStaticCtor(void) const;

    JClass *getNestHost(FExec *ctx);

    uint16_t getNestMembersCount(void) const;
    JClass *getNestMember(FExec *ctx, uint16_t index);

    uint16_t hasStaticObjField(void) const;
    FieldValue *getStaticField(class FExec *ctx, ConstField *field) const;
    FieldValue *getStaticField(class FExec *ctx, const char *name) const;
    FieldValue *getStaticFieldByIndex(uint32_t index) const;

    StaticInitStatus getStaticInitStatus(void) const;
    void staticInitialized(void);
    bool initStaticFields(FExec *ctx);
    void clearStaticFields(void);
private:
    ClassLoader(void);
    ClassLoader(const ClassLoader &) = delete;
    void operator=(const ClassLoader &) = delete;

    bool load(class FExec *ctx, FlintAPI::IO::FileHandle file);
    static CodeAttribute *readAttributeCode(class FExec *ctx, void *file);
public:
    static ClassLoader *load(class FExec *ctx, const char *clsName, uint16_t length = 0xFFFF);

    ~ClassLoader(void);
};

#endif /* __FLINT_CLASS_LOADER_H */
