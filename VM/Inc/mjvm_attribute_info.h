
#ifndef __MJVM_ATTRIBUTE_INFO_H
#define __MJVM_ATTRIBUTE_INFO_H

#include "mjvm_const_pool.h"

class Mjvm;

typedef enum : uint8_t {
    ATTRIBUTE_CONSTANT_VALUE = 0,
    ATTRIBUTE_CODE,
    ATTRIBUTE_STACK_MAP_TABLE,
    ATTRIBUTE_EXCEPTIONS,
    ATTRIBUTE_INNER_CLASSES,
    ATTRIBUTE_ENCLOSING_METHOD,
    ATTRIBUTE_SYNTHETIC,
    ATTRIBUTE_SIGNATURE,
    ATTRIBUTE_SOURCE_FILE,
    ATTRIBUTE_SOURCE_DEBUG_EXTENSION,
    ATTRIBUTE_LINE_NUMBER_TABLE,
    ATTRIBUTE_LOCAL_VARIABLE_TABLE,
    ATTRIBUTE_LOCAL_VARIABLE_TYPE_TABLE,
    ATTRIBUTE_DEPRECATED,
    ATTRIBUTE_RUNTIME_VISIBLE_ANNOTATIONS,
    ATTRIBUTE_RUNTIME_INVISIBLE_ANNOTATIONS,
    ATTRIBUTE_RUNTIME_VISIBLE_PARAMETER_ANNOTATIONS,
    ATTRIBUTE_RUNTIME_INVISIBLE_PARAMETER_ANNOTATIONS,
    ATTRIBUTE_ANNOTATION_DEFAULT,
    ATTRIBUTE_BOOTSTRAP_METHODS,
    ATTRIBUTE_NEST_MEMBERS,
    ATTRIBUTE_NATIVE,
    ATTRIBUTE_UNKNOW = 0xFF
} MjvmAttributeType;

class MjvmAttribute {
protected:
    MjvmAttribute *next;
public:
    const MjvmAttributeType attributeType;
private:
    MjvmAttribute(const MjvmAttribute &) = delete;
    void operator=(const MjvmAttribute &) = delete;

    friend class MjvmClassLoader;
    friend class MjvmMethodInfo;
    friend class MjvmCodeAttribute;
protected:
    MjvmAttribute(MjvmAttributeType type);
public:
    static MjvmAttributeType parseAttributeType(const MjvmConstUtf8 &name);

    virtual ~MjvmAttribute(void) = 0;
};

class MjvmExecution;

typedef bool (*MjvmNativeMethodPtr)(MjvmExecution &execution);

class MjvmNativeAttribute : public MjvmAttribute {
public:
    MjvmNativeMethodPtr nativeMethod;
private:
    MjvmNativeAttribute(MjvmNativeMethodPtr nativeMethod);
    MjvmNativeAttribute(const MjvmNativeAttribute &) = delete;
    void operator=(const MjvmNativeAttribute &) = delete;

    friend class MjvmClassLoader;

    ~MjvmNativeAttribute(void);
};

class MjvmExceptionTable {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const uint16_t catchType;
private:
    MjvmExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType);
    MjvmExceptionTable(const MjvmExceptionTable &) = delete;
    void operator=(const MjvmExceptionTable &) = delete;

    friend class MjvmClassLoader;
};

class MjvmCodeAttribute : public MjvmAttribute {
public:
    const uint16_t maxStack;
    const uint16_t maxLocals;
    const uint32_t codeLength;
    const uint16_t exceptionTableLength;
    const uint8_t *code;
private:
    MjvmExceptionTable *exceptionTable;
    MjvmAttribute *attributes;

    MjvmCodeAttribute(uint16_t maxStack, uint16_t maxLocals);
    MjvmCodeAttribute(const MjvmCodeAttribute &) = delete;
    void operator=(const MjvmCodeAttribute &) = delete;

    void setCode(uint8_t *code, uint32_t length);
    void setExceptionTable(MjvmExceptionTable *exceptionTable, uint16_t length);
    void addAttribute(MjvmAttribute *attribute);

    ~MjvmCodeAttribute(void);

    friend class MjvmClassLoader;
public:
    MjvmExceptionTable &getException(uint16_t index) const;
};

class MjvmBootstrapMethod {
public:
    const uint16_t bootstrapMethodRef;
    const uint16_t numBootstrapArguments;

    uint16_t getBootstrapArgument(uint16_t index) const;
private:
    uint16_t bootstrapArguments[];

    MjvmBootstrapMethod(uint16_t bootstrapMethodRef, uint16_t numBootstrapArguments);
    MjvmBootstrapMethod(const MjvmBootstrapMethod &) = delete;
    void operator=(const MjvmBootstrapMethod &) = delete;

    friend class MjvmClassLoader;
};

class AttributeBootstrapMethods : public MjvmAttribute {
public:
    const uint16_t numBootstrapMethods;
private:
    MjvmBootstrapMethod **bootstrapMethods;

    AttributeBootstrapMethods(uint16_t numBootstrapMethods);
    AttributeBootstrapMethods(const AttributeBootstrapMethods &) = delete;
    void operator=(const AttributeBootstrapMethods &) = delete;

    MjvmBootstrapMethod &getBootstrapMethod(uint16_t index);
    void setBootstrapMethod(uint16_t index, MjvmBootstrapMethod &bootstrapMethod);

    ~AttributeBootstrapMethods(void);

    friend class MjvmClassLoader;
};

#endif /* __MJVM_ATTRIBUTE_INFO_H */
