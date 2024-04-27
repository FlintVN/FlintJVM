
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
    ATTRIBUTE_BOOTSTRAP_METHODS,
    ATTRIBUTE_NEST_MEMBERS,
    ATTRIBUTE_NATIVE,
    ATTRIBUTE_UNKNOW = 0xFF
} AttributeType;

class AttributeInfo {
public:
    const AttributeType attributeType;
private:
    AttributeInfo(const AttributeInfo &) = delete;
    void operator=(const AttributeInfo &) = delete;
protected:
    AttributeInfo(AttributeType type);
public:
    static AttributeType parseAttributeType(const ConstUtf8 &name);

    virtual ~AttributeInfo(void) = 0;
};

class AttributeRaw : public AttributeInfo {
public:
    const uint32_t length;
private:
    uint8_t raw[];

    AttributeRaw(AttributeType type, uint16_t length);
    AttributeRaw(const AttributeRaw &) = delete;
    void operator=(const AttributeRaw &) = delete;

    ~AttributeRaw(void);

    friend class ClassLoader;
public:
    const uint8_t *getRaw(void) const;
};

class Execution;

typedef bool (* const NativeMethodPtr)(Execution &execution);

class AttributeNative : public AttributeInfo {
public:
    NativeMethodPtr nativeMethod;
private:
    AttributeNative(NativeMethodPtr nativeMethod);
    AttributeNative(const AttributeNative &) = delete;
    void operator=(const AttributeNative &) = delete;

    friend class ClassLoader;

    ~AttributeNative(void);
};

class ExceptionTable {
public:
    const uint16_t startPc;
    const uint16_t endPc;
    const uint16_t handlerPc;
    const uint16_t catchType;
private:
    ExceptionTable(uint16_t startPc, uint16_t endPc, uint16_t handlerPc, uint16_t catchType);
    ExceptionTable(const ExceptionTable &) = delete;
    void operator=(const ExceptionTable &) = delete;

    friend class ClassLoader;
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

    AttributeCode(uint16_t maxStack, uint16_t maxLocals);
    AttributeCode(const AttributeCode &) = delete;
    void operator=(const AttributeCode &) = delete;

    void setCode(uint8_t *code, uint32_t length);
    void setExceptionTable(ExceptionTable *exceptionTable, uint16_t length);
    void setAttributes(AttributeInfo **attributes, uint16_t length);

    ~AttributeCode(void);

    friend class ClassLoader;
public:
    const ExceptionTable &getException(uint16_t index) const;
    const AttributeInfo &getAttributes(uint16_t index) const;
};

class LineNumber {
public:
    const uint16_t startPc;
    const uint16_t lineNumber;
private:
    LineNumber(uint16_t startPc, uint16_t lineNumber);
    LineNumber(const LineNumber &) = delete;
    void operator=(const LineNumber &) = delete;

    friend class ClassLoader;
};

class AttributeLineNumberTable : public AttributeInfo {
public:
    const uint16_t LineNumberLenght;
private:
    LineNumber lineNumberTable[];

    AttributeLineNumberTable(uint16_t length);
    AttributeLineNumberTable(const AttributeLineNumberTable &) = delete;
    void operator=(const AttributeLineNumberTable &) = delete;

    ~AttributeLineNumberTable(void);

    friend class ClassLoader;
public:
    const LineNumber &getLineNumber(uint16_t index) const;
};

class LocalVariable {
public:
    const uint16_t startPc;
    const uint16_t length;
    const uint16_t index;
    const ConstUtf8 &name;
    const ConstUtf8 &descriptor;
private:
    LocalVariable(uint16_t startPc, uint16_t length, const ConstUtf8 &name, const ConstUtf8 &descriptor, uint16_t index);
    LocalVariable(const LocalVariable &) = delete;
    void operator=(const LocalVariable &) = delete;

    friend class ClassLoader;
};

class AttributeLocalVariableTable : public AttributeInfo {
public:
    const uint16_t localVariableLength;
private:
    LocalVariable lineNumberTable[];

    AttributeLocalVariableTable(uint16_t length);
    AttributeLocalVariableTable(const AttributeLocalVariableTable &) = delete;
    void operator=(const AttributeLocalVariableTable &) = delete;

    ~AttributeLocalVariableTable(void);

    friend class ClassLoader;
public:
    const LocalVariable &getLocalVariable(uint16_t index) const;
};

class BootstrapMethod {
public:
    const uint16_t bootstrapMethodRef;
    const uint16_t numBootstrapArguments;

    const uint16_t getBootstrapArgument(uint16_t index) const;
private:
    uint16_t bootstrapArguments[];

    BootstrapMethod(uint16_t bootstrapMethodRef, uint16_t numBootstrapArguments);
    BootstrapMethod(const BootstrapMethod &) = delete;
    void operator=(const BootstrapMethod &) = delete;

    friend class ClassLoader;
};

class AttributeBootstrapMethods : public AttributeInfo {
public:
    const uint16_t numBootstrapMethods;
private:
    const BootstrapMethod **bootstrapMethods;

    AttributeBootstrapMethods(uint16_t numBootstrapMethods);
    AttributeBootstrapMethods(const AttributeBootstrapMethods &) = delete;
    void operator=(const AttributeBootstrapMethods &) = delete;

    const BootstrapMethod &getBootstrapMethod(uint16_t index);
    void setBootstrapMethod(uint16_t index, const BootstrapMethod &bootstrapMethod);

    ~AttributeBootstrapMethods(void);

    friend class ClassLoader;
};

#endif /* __MJVM_ATTRIBUTE_INFO_H */
