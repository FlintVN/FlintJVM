
#ifndef __FLINT_ATTRIBUTE_INFO_H
#define __FLINT_ATTRIBUTE_INFO_H

#include "flint_const_pool.h"

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
} FlintAttributeType;

class FlintAttribute {
public:
    const FlintAttributeType attributeType;
private:
    FlintAttribute(const FlintAttribute &) = delete;
    void operator=(const FlintAttribute &) = delete;

    friend class FlintClassLoader;
    friend class FlintMethodInfo;
    friend class FlintCodeAttribute;
protected:
    FlintAttribute(FlintAttributeType type);
public:
    static FlintAttributeType parseAttributeType(const FlintConstUtf8 &name);

    virtual ~FlintAttribute(void) = 0;
};

typedef void (*FlintNativeMethodPtr)(class FlintExecution &execution);

class FlintExceptionTable {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const uint16_t catchType;
private:
    FlintExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType);
    FlintExceptionTable(const FlintExceptionTable &) = delete;
    void operator=(const FlintExceptionTable &) = delete;

    friend class FlintClassLoader;
};

class FlintCodeAttribute : public FlintAttribute {
public:
    const uint16_t maxStack;
    const uint16_t maxLocals;
    const uint32_t codeLength;
    const uint16_t exceptionTableLength;
    const uint8_t *code;
private:
    FlintExceptionTable *exceptionTable;

    FlintCodeAttribute(uint16_t maxStack, uint16_t maxLocals);
    FlintCodeAttribute(const FlintCodeAttribute &) = delete;
    void operator=(const FlintCodeAttribute &) = delete;

    void setCode(uint8_t *code, uint32_t length);
    void setExceptionTable(FlintExceptionTable *exceptionTable, uint16_t length);

    ~FlintCodeAttribute(void);

    friend class FlintClassLoader;
public:
    FlintExceptionTable *getException(uint16_t index) const;
};

#endif /* __FLINT_ATTRIBUTE_INFO_H */
