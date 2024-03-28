
#ifndef __MJVM_ATTRIBUTE_INFO_H
#define __MJVM_ATTRIBUTE_INFO_H

#include "mjvm_const_pool.h"

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
    ATTRIBUTE_BOOTSTRAP_METHODS
} AttributeType;

class AttributeInfo {
public:
    const AttributeType attributeType;
private:
    AttributeInfo(const AttributeInfo &) = delete;
    void operator=(const AttributeInfo &) = delete;
protected:
    AttributeInfo(AttributeType attributeType);
public:
    static AttributeType parseAttributeType(const ConstUtf8 &name);

    virtual ~AttributeInfo(void) = 0;
};

class ExceptionTable {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const ConstUtf8 &catchType;

    ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, const ConstUtf8 &catchType);
private:
    ExceptionTable(const ExceptionTable &) = delete;
    void operator=(const ExceptionTable &) = delete;
};

class AttributeCode : public AttributeInfo {
public:
    const uint16_t maxStack;
    const uint16_t maxLocals;
    const uint32_t codeLength;
    const uint16_t exceptionTableLength;
    const uint16_t attributesCount;
    const uint8_t *code;
private:
    const ExceptionTable *exceptionTable;
    const AttributeInfo **attributes;

    AttributeCode(const AttributeCode &) = delete;
    void operator=(const AttributeCode &) = delete;

    void setCode(uint8_t *code, uint32_t length);
    void setExceptionTable(ExceptionTable *exceptionTable, uint16_t length);
    void setAttributes(AttributeInfo **attributes, uint16_t length);

    friend class ClassLoader;
public:
    AttributeCode(uint16_t maxStack, uint16_t maxLocals);

    const ExceptionTable &getException(uint16_t index) const;
    const AttributeInfo &getAttributes(uint16_t index) const;

    ~AttributeCode(void);
};

#endif /* __MJVM_ATTRIBUTE_INFO_H */
