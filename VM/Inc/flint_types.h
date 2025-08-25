
#ifndef __FLINT_TYPE_H
#define __FLINT_TYPE_H

#include "flint_std_types.h"

#define NULL_PTR        0

typedef enum : uint8_t {
    CONST_UTF8 = 1,
    CONST_INTEGER = 3,
    CONST_FLOAT = 4,
    CONST_LONG = 5,
    CONST_DOUBLE = 6,
    CONST_CLASS = 7,
    CONST_STRING = 8,
    CONST_FIELD = 9,
    CONST_METHOD = 10,
    CONST_INTERFACE_METHOD = 11,
    CONST_NAME_AND_TYPE = 12,
    CONST_METHOD_HANDLE = 15,
    CONST_METHOD_TYPE = 16,
    CONST_INVOKE_DYNAMIC = 18,
    CONST_UNKOWN = 0xFF,
} FlintConstPoolTag;

typedef enum : uint8_t {
    REF_GET_FIELD = 1,
    REF_GET_STATIC = 2,
    REF_PUT_FIELD = 3,
    REF_PUT_STATIC = 4,
    REF_INVOKE_VIRTUAL = 5,
    REF_INVOKE_STATIC = 6,
    REF_INVOKE_SPECIAL = 7,
    REF_NEW_INVOKE_SPECIAL = 8,
    REF_INVOKE_INTERFACE = 9,
} FlintReferenceKind;

typedef enum : uint16_t {
    CLASS_PUBLIC = 0x0001,
    CLASS_FINAL = 0x0010,
    CLASS_SUPER = 0x0020,
    CLASS_INTERFACE = 0x0200,
    CLASS_ABSTRACT = 0x0400,
    CLASS_SYNTHETIC = 0x1000,
    CLASS_ANNOTATION = 0x2000,
    CLASS_ENUM = 0x4000,
} FlintClassAccessFlag;

typedef enum : uint16_t {
    FIELD_PUBLIC = 0x0001,
    FIELD_PRIVATE = 0x0002,
    FIELD_PROTECTED = 0x0004,
    FIELD_STATIC = 0x0008,
    FIELD_FINAL = 0x0010,
    FIELD_VOLATILE = 0x0040,
    FIELD_TRANSIENT = 0x0080,
    FIELD_SYNTHETIC = 0x1000,
    FIELD_ENUM = 0x4000,
    FIELD_UNLOAD = 0x8000,
} FlintFieldAccessFlag;

typedef enum : uint16_t {
    METHOD_PUBLIC = 0x0001,
    METHOD_PRIVATE = 0x0002,
    METHOD_PROTECTED = 0x0004,
    METHOD_STATIC = 0x0008,
    METHOD_FINAL = 0x0010,
    METHOD_SYNCHRONIZED = 0x0020,
    METHOD_BRIDGE = 0x0040,
    METHOD_VARARGS = 0x0080,
    METHOD_NATIVE = 0x0100,
    METHOD_ABSTRACT = 0x0400,
    METHOD_STRICT = 0x0800,
    METHOD_SYNTHETIC = 0x1000,
    METHOD_UNLOADED = 0x2000,
} FlintMethodAccessFlag;

typedef enum : uint16_t {
    INNER_CLASS_PUBLIC = 0x0001,
    INNER_CLASS_PRIVATE = 0x0002,
    INNER_CLASS_PROTECTED = 0x0004,
    INNER_CLASS_STATIC = 0x0008,
    INNER_CLASS_FINAL = 0x0010,
    INNER_CLASS_INTERFACE = 0x0200,
    INNER_CLASS_ABSTRACT = 0x0400,
    INNER_CLASS_SYNTHETIC = 0x1000,
    INNER_CLASS_ANNOTATION = 0x2000,
    INNER_CLASS_ENUM = 0x4000,
} FlintInnerClassAccessFlag;

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

typedef enum : uint8_t {
    ERR_OK = 0,
    ERR_THROW,
    ERR_LOCK_FAIL,
    ERR_LOCK_LIMIT,
    ERR_OUT_OF_MEMORY,
    ERR_STACK_OVERFLOW,
    ERR_CLASS_LOAD_FAIL,
    ERR_CLASS_NOT_FOUND,
    ERR_FIELD_NOT_FOUND,
    ERR_METHOD_NOT_FOUND,
    ERR_TERMINATE_REQUEST,
    ERR_STATIC_CTOR_IS_RUNNING,
    ERR_VM_ERROR,
} FlintError;

template <class T>
class FlintResult {
public:
    FlintError err;
private:
    uint16_t errMsgLen;
public:
    union {
        T *value;
        const char *errMsg;
    };
public:
    FlintResult(T *value) : err(ERR_OK), value(value) {

    }

    FlintResult(FlintError errorCode, const char *msg = NULL_PTR, uint16_t length = 0) : err(errorCode), errMsgLen(length), errMsg(msg) {

    }

    const char *getErrorMsg(void) const {
        if(err == ERR_OK)
            return NULL_PTR;
        return errMsg;
    }

    uint32_t getErrorMsgLength(void) const {
        return errMsgLen;
    }
};

template <>
class FlintResult<bool> {
public:
    FlintError err;
private:
    uint16_t errMsgLen;
public:
    union {
        bool value;
        const char *errMsg;
    };
public:
    FlintResult(bool value) : err(ERR_OK), value(value) {

    }

    FlintResult(FlintError errorCode, const char *msg = NULL_PTR, uint16_t length = 0) : err(errorCode), errMsgLen(length), errMsg(msg) {

    }

    const char *getErrorMsg(void) const {
        if(err == ERR_OK)
            return NULL_PTR;
        return errMsg;
    }

    uint32_t getErrorMsgLength(void) const {
        return errMsgLen;
    }
};

template <>
class FlintResult<void> {
public:
    FlintError err;
private:
    uint16_t errMsgLen;
    const char *errMsg;
public:
    FlintResult(FlintError errorCode, const char *msg = NULL_PTR, uint16_t length = 0) : err(errorCode), errMsgLen(length), errMsg(msg) {

    }

    const char *getErrorMsg(void) const {
        if(err == ERR_OK)
            return NULL_PTR;
        return errMsg;
    }

    uint32_t getErrorMsgLength(void) const {
        return errMsgLen;
    }
};

typedef FlintError (*FlintNativeMethodPtr)(class FlintExecution *exec);

#endif /* __FLINT_TYPE_H */
