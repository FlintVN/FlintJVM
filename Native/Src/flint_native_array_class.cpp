
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_const_name_base.h"
#include "flint_native_array_class.h"
#include "flint_java_boolean.h"
#include "flint_java_byte.h"
#include "flint_java_character.h"
#include "flint_java_short.h"
#include "flint_java_integer.h"
#include "flint_java_float.h"
#include "flint_java_long.h"
#include "flint_java_double.h"
#include "flint_throw_support.h"

static FlintError checkIsArray(FlintExecution *exec, FlintJavaObject *obj) {
    if(obj == NULL_PTR)
        return throwNullPointerException(exec);
    if(obj->dimensions < 1)
        return throwIllegalArgumentException(exec, "Argument is not an array");
    return ERR_OK;
}

static FlintError checkIsClassType(FlintExecution *exec, FlintJavaObject *obj) {
    if(obj == NULL_PTR)
        return throwNullPointerException(exec);
    else if(obj->type != *(FlintConstUtf8 *)classClassName)
        return throwIllegalArgumentException(exec);
    return ERR_OK;
}

static FlintError checkIndex(FlintExecution *exec, FlintJavaObject *obj, int32_t index) {
    int32_t length = obj->size / obj->parseTypeSize();
    if((index < 0) || (index >= length))
        return throwArrayIndexOutOfBoundsException(exec, index, length);
    return ERR_OK;
}

static FlintError checkLength(FlintExecution *exec, int32_t size) {
    if(size < 0)
        return throwNegativeArraySizeException(exec);
    return ERR_OK;
}

static FlintError checkDimensions(FlintExecution *exec, FlintInt32Array *dimensions) {
    if(
        (dimensions == NULL_PTR) ||
        (dimensions->dimensions != 1) ||
        (dimensions->type != *(FlintConstUtf8 *)integerPrimTypeName) ||
        (dimensions->getLength() == 0) ||
        (dimensions->getLength() > 255)
    ) {
        return throwIllegalArgumentException(exec);
    }
    return ERR_OK;
}

static FlintError checkIsArrayOfPrimitiveType(FlintExecution *exec, FlintJavaObject *obj) {
    RETURN_IF_ERR(checkIsArray(exec, obj));
    if((obj->dimensions != 1) || !FlintJavaObject::isPrimType(obj->type))
        return throwIllegalArgumentException(exec, "Argument is not an array of primitive type");
    return ERR_OK;
}

static FlintError nativeGetLength(FlintExecution *exec) {
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArray(exec, obj));
    exec->stackPushInt32(obj->size / obj->parseTypeSize());
    return ERR_OK;
}

static FlintError nativeGet(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArray(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    if(obj->dimensions == 1 && FlintJavaObject::isPrimType(obj->type)) {
        switch(obj->type.text[0]) {
            case 'B': { /* byte */
                auto val = exec->flint.newByte(((FlintInt8Array *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            case 'Z': { /* boolean */
                auto val = exec->flint.newBoolean(((FlintInt8Array *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            case 'C': { /* char */
                auto val = exec->flint.newCharacter(((FlintInt16Array *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            case 'S': { /* short */
                auto val = exec->flint.newShort(((FlintInt16Array *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            case 'I': { /* integer */
                auto val = exec->flint.newInteger(((FlintInt32Array *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            case 'F': { /* float */
                auto val = exec->flint.newFloat(((FlintFloatArray *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            case 'D': { /* double */
                auto val = exec->flint.newDouble(((FlintDoubleArray *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
            default: { /* long */
                auto val = exec->flint.newLong(((FlintInt64Array *)obj)->getData()[index]);
                if(val.err != ERR_OK)
                    return checkAndThrowForFlintError(exec, val.err, val.getErrorMsg(), val.getErrorMsgLength());
                exec->stackPushObject(val.value);
                return ERR_OK;
            }
        }
    }
    else {
        exec->stackPushObject(((FlintObjectArray *)obj)->getData()[index]);
        return ERR_OK;
    }
}

static FlintError nativeGetBoolean(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    if(obj->type.text[0] != 'Z')
        return throwIllegalArgumentException(exec, "Argument type mismatch");
    exec->stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
    return ERR_OK;
}

static FlintError nativeGetByte(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    if(obj->type.text[0] != 'B')
        return throwIllegalArgumentException(exec, "Argument type mismatch");
    exec->stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
    return ERR_OK;
}

static FlintError nativeGetChar(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    if(obj->type.text[0] != 'C')
        return throwIllegalArgumentException(exec, "Argument type mismatch");
    exec->stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
    return ERR_OK;
}

static FlintError nativeGetShort(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            exec->stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'S': /* short */
            exec->stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeGetInt(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            exec->stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            exec->stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            exec->stackPushInt32(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeGetLong(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            exec->stackPushInt64(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            exec->stackPushInt64(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            exec->stackPushInt64(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'J': /* long */
            exec->stackPushInt64(((FlintInt64Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeGetFloat(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            exec->stackPushFloat(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            exec->stackPushFloat(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            exec->stackPushFloat(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'F': /* float */
            exec->stackPushFloat(((FlintFloatArray *)obj)->getData()[index]);
            return ERR_OK;
        case 'J': /* long */
            exec->stackPushFloat(((FlintInt64Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeGetDouble(FlintExecution *exec) {
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            exec->stackPushDouble(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            exec->stackPushDouble(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            exec->stackPushDouble(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'F': /* float */
            exec->stackPushDouble(((FlintFloatArray *)obj)->getData()[index]);
            return ERR_OK;
        case 'J': /* long */
            exec->stackPushDouble(((FlintInt64Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'D': /* double */
            exec->stackPushDouble(((FlintDoubleArray *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSet(FlintExecution *exec) {
    FlintJavaObject *value = exec->stackPopObject();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArray(exec, obj));
    if((obj->dimensions == 1) && (FlintJavaObject::isPrimType(obj->type))) {
        if((!value) || (value->dimensions != 0))
            return throwIllegalArgumentException(exec, "Argument type mismatch");
        switch(obj->type.text[0]) {
            case 'Z': { /* boolean */
                if(value->type == *(FlintConstUtf8 *)booleanClassName) {
                    ((FlintInt8Array *)obj)->getData()[index] = (int8_t)((FlintJavaBoolean *)value)->getValue();
                    return ERR_OK;
                }
                return throwIllegalArgumentException(exec, "Argument type mismatch");
            }
            case 'B': { /* byte */
                if(value->type == *(FlintConstUtf8 *)byteClassName) {
                    ((FlintInt8Array *)obj)->getData()[index] = (int8_t)((FlintJavaByte *)value)->getValue();
                    return ERR_OK;
                }
                return throwIllegalArgumentException(exec, "Argument type mismatch");
            }
            case 'C': { /* char */
                if(value->type == *(FlintConstUtf8 *)characterClassName) {
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaCharacter *)value)->getValue();
                    return ERR_OK;
                }
                return throwIllegalArgumentException(exec, "Argument type mismatch");
            }
            case 'S': { /* short */
                if(value->type == *(FlintConstUtf8 *)byteClassName)
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaByte *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)shortClassName)
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaShort *)value)->getValue();
                else
                    return throwIllegalArgumentException(exec, "Argument type mismatch");
                return ERR_OK;
            }
            case 'I': { /* integer */
                if(value->type == *(FlintConstUtf8 *)byteClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)characterClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)shortClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)integerClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else
                    return throwIllegalArgumentException(exec, "Argument type mismatch");
                return ERR_OK;
            }
            case 'F': { /* float */
                if(value->type == *(FlintConstUtf8 *)byteClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)characterClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)shortClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)integerClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)floatClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaFloat *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)longClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaLong *)value)->getValue();
                else
                    return throwIllegalArgumentException(exec, "Argument type mismatch");
                return ERR_OK;
            }
            case 'D': { /* double */
                if(value->type == *(FlintConstUtf8 *)byteClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)characterClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)shortClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)integerClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)floatClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaFloat *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)longClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaLong *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)doubleClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaDouble *)value)->getValue();
                else
                    return throwIllegalArgumentException(exec, "Argument type mismatch");
                return ERR_OK;
            }
            default: { /* long */
                if(value->type == *(FlintConstUtf8 *)byteClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)characterClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)shortClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)integerClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)longClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaLong *)value)->getValue();
                else
                    return throwIllegalArgumentException(exec, "Argument type mismatch");
                return ERR_OK;
            }
        }
    }
    else {
        RETURN_IF_ERR(checkIndex(exec, obj, index));
        ((FlintObjectArray *)obj)->getData()[index] = value;
        return ERR_OK;
    }
}

static FlintError nativeSetBoolean(FlintExecution *exec) {
    int8_t value = exec->stackPopInt32();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    if(obj->type.text[0] != 'Z')
        return throwIllegalArgumentException(exec, "Argument type mismatch");
    ((FlintInt8Array *)obj)->getData()[index] = !!value;
    return ERR_OK;
}

static FlintError nativeSetByte(FlintExecution *exec) {
    int8_t value = exec->stackPopInt32();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            ((FlintInt8Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'S': /* short */
            ((FlintInt16Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSetChar(FlintExecution *exec) {
    int16_t value = exec->stackPopInt32();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'C': /* char */
            ((FlintInt16Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSetShort(FlintExecution *exec) {
    int16_t value = exec->stackPopInt32();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'S': /* short */
            ((FlintInt16Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSetInt(FlintExecution *exec) {
    int32_t value = exec->stackPopInt32();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSetLong(FlintExecution *exec) {
    int64_t value = exec->stackPopInt64();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSetFloat(FlintExecution *exec) {
    float value = exec->stackPopFloat();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    switch(obj->type.text[0]) {
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(exec, "Argument type mismatch");
    }
}

static FlintError nativeSetDouble(FlintExecution *exec) {
    double value = exec->stackPopDouble();
    int32_t index = exec->stackPopInt32();
    FlintJavaObject *obj = exec->stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(exec, obj));
    RETURN_IF_ERR(checkIndex(exec, obj, index));
    if(obj->type.text[0] != 'D')
        return throwIllegalArgumentException(exec, "Argument type mismatch");
    ((FlintDoubleArray *)obj)->getData()[index] = value;
    return ERR_OK;
}

static FlintError nativeNewArray(FlintExecution *exec) {
    int32_t length = exec->stackPopInt32();
    FlintJavaClass *componentType = (FlintJavaClass *)exec->stackPopObject();
    RETURN_IF_ERR(checkIsClassType(exec, componentType));
    RETURN_IF_ERR(checkLength(exec, length));
    uint32_t dimensions;
    auto typeName = componentType->getBaseTypeName(exec->flint, &dimensions);
    if(typeName.err != ERR_OK)
        return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
    if(typeName.value == (FlintConstUtf8 *)voidPrimTypeName) /* void */
        return throwIllegalArgumentException(exec);
    uint8_t atype = FlintJavaObject::isPrimType(*typeName.value);
    uint8_t typeSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
    auto array = exec->flint.newObject(typeSize * length, typeName.value, dimensions + 1);
    if(array.err != ERR_OK)
        return checkAndThrowForFlintError(exec, array.err, array.getErrorMsg(), array.getErrorMsgLength());
    memset((void *)&array.value->getFields(), 0, array.value->size);
    exec->stackPushObject(array.value);
    return ERR_OK;
}

static FlintError nativeMultiNewArray(FlintExecution *exec) {
    FlintInt32Array *dimensions = (FlintInt32Array *)exec->stackPopObject();
    FlintJavaClass *componentType = (FlintJavaClass *)exec->stackPopObject();
    RETURN_IF_ERR(checkIsClassType(exec, componentType));
    RETURN_IF_ERR(checkDimensions(exec, dimensions));
    uint32_t endDims;
    auto typeName = componentType->getBaseTypeName(exec->flint, &endDims);
    if(typeName.err != ERR_OK)
        return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
    if(typeName.value == (FlintConstUtf8 *)voidPrimTypeName) /* void */
        return throwIllegalArgumentException(exec);
    if((dimensions->getLength() + endDims) > 255)
        return throwIllegalArgumentException(exec);
    auto array = exec->flint.newMultiArray(typeName.value, dimensions->getData(), dimensions->getLength() + endDims, endDims + 1);
    if(array.err != ERR_OK)
        return checkAndThrowForFlintError(exec, array.err, array.getErrorMsg(), array.getErrorMsgLength());
    exec->stackPushObject(array.value);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x09\x00\x40\xF0""getLength",     "\x15\x00\x49\x76""(Ljava/lang/Object;)I",                    nativeGetLength),
    NATIVE_METHOD("\x03\x00\x14\x97""get",           "\x27\x00\xB7\x22""(Ljava/lang/Object;I)Ljava/lang/Object;",  nativeGet),
    NATIVE_METHOD("\x0A\x00\xB6\x78""getBoolean",    "\x16\x00\x5F\x6C""(Ljava/lang/Object;I)Z",                   nativeGetBoolean),
    NATIVE_METHOD("\x07\x00\x69\x4D""getByte",       "\x16\x00\x5F\x66""(Ljava/lang/Object;I)B",                   nativeGetByte),
    NATIVE_METHOD("\x07\x00\x76\x2A""getChar",       "\x16\x00\x9E\xA6""(Ljava/lang/Object;I)C",                   nativeGetChar),
    NATIVE_METHOD("\x08\x00\x4B\x7E""getShort",      "\x16\x00\x9F\x6A""(Ljava/lang/Object;I)S",                   nativeGetShort),
    NATIVE_METHOD("\x06\x00\x5D\x9A""getInt",        "\x16\x00\x1E\xA1""(Ljava/lang/Object;I)I",                   nativeGetInt),
    NATIVE_METHOD("\x07\x00\x00\xC0""getLong",       "\x16\x00\x5E\xA0""(Ljava/lang/Object;I)J",                   nativeGetLong),
    NATIVE_METHOD("\x08\x00\x4A\xBD""getFloat",      "\x16\x00\x5E\xA5""(Ljava/lang/Object;I)F",                   nativeGetFloat),
    NATIVE_METHOD("\x09\x00\x04\x99""getDouble",     "\x16\x00\xDF\x64""(Ljava/lang/Object;I)D",                   nativeGetDouble),
    NATIVE_METHOD("\x03\x00\x54\x93""set",           "\x28\x00\x12\x5A""(Ljava/lang/Object;ILjava/lang/Object;)V", nativeSet),
    NATIVE_METHOD("\x0A\x00\xF6\x38""setBoolean",    "\x17\x00\x38\x52""(Ljava/lang/Object;IZ)V",                  nativeSetBoolean),
    NATIVE_METHOD("\x07\x00\x3D\x4C""setByte",       "\x17\x00\xB8\x55""(Ljava/lang/Object;IB)V",                  nativeSetByte),
    NATIVE_METHOD("\x07\x00\x22\x2B""setChar",       "\x17\x00\xE9\x95""(Ljava/lang/Object;IC)V",                  nativeSetChar),
    NATIVE_METHOD("\x08\x00\x4B\x81""setShort",      "\x17\x00\xE8\x50""(Ljava/lang/Object;IS)V",                  nativeSetShort),
    NATIVE_METHOD("\x06\x00\x5E\x8E""setInt",        "\x17\x00\xC9\x97""(Ljava/lang/Object;II)V",                  nativeSetInt),
    NATIVE_METHOD("\x07\x00\x54\xC1""setLong",       "\x17\x00\x39\x97""(Ljava/lang/Object;IJ)V",                  nativeSetLong),
    NATIVE_METHOD("\x08\x00\x4A\x42""setFloat",      "\x17\x00\xF9\x94""(Ljava/lang/Object;IF)V",                  nativeSetFloat),
    NATIVE_METHOD("\x09\x00\xFB\x99""setDouble",     "\x17\x00\x58\x54""(Ljava/lang/Object;ID)V",                  nativeSetDouble),
    NATIVE_METHOD("\x08\x00\x68\xCF""newArray",      "\x26\x00\xAA\x4D""(Ljava/lang/Class;I)Ljava/lang/Object;",   nativeNewArray),
    NATIVE_METHOD("\x0D\x00\xB6\x54""multiNewArray", "\x27\x00\x66\xF4""(Ljava/lang/Class;[I)Ljava/lang/Object;",  nativeMultiNewArray),
};

const FlintNativeClass ARRAY_CLASS = NATIVE_CLASS(arrayClassName, methods);
