
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

static FlintError checkIsArray(FlintExecution &execution, FlintJavaObject *obj) {
    if(obj == NULL)
        return throwNullPointerException(execution);
    if(obj->dimensions < 1)
        return throwIllegalArgumentException(execution, "Argument is not an array");
    return ERR_OK;
}

static FlintError checkIsClassType(FlintExecution &execution, FlintJavaObject *obj) {
    if(obj == NULL)
        return throwNullPointerException(execution);
    else if(obj->type != *(FlintConstUtf8 *)classClassName)
        return throwIllegalArgumentException(execution);
    return ERR_OK;
}

static FlintError checkIndex(FlintExecution &execution, FlintJavaObject *obj, int32_t index) {
    int32_t length = obj->size / obj->parseTypeSize();
    if((index < 0) || (index >= length))
        return throwArrayIndexOutOfBoundsException(execution, index, length);
    return ERR_OK;
}

static FlintError checkLength(FlintExecution &execution, int32_t size) {
    if(size < 0)
        return throwNegativeArraySizeException(execution);
    return ERR_OK;
}

static FlintError checkDimensions(FlintExecution &execution, FlintInt32Array *dimensions) {
    if(
        (dimensions == NULL) ||
        (dimensions->dimensions != 1) ||
        (dimensions->type != *(FlintConstUtf8 *)integerPrimTypeName) ||
        (dimensions->getLength() == 0) ||
        (dimensions->getLength() > 255)
    ) {
        return throwIllegalArgumentException(execution);
    }
    return ERR_OK;
}

static FlintError checkIsArrayOfPrimitiveType(FlintExecution &execution, FlintJavaObject *obj) {
    RETURN_IF_ERR(checkIsArray(execution, obj));
    if((obj->dimensions != 1) || !FlintJavaObject::isPrimType(obj->type))
        return throwIllegalArgumentException(execution, "Argument is not an array of primitive type");
    return ERR_OK;
}

static FlintError nativeGetLength(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArray(execution, obj));
    execution.stackPushInt32(obj->size / obj->parseTypeSize());
    return ERR_OK;
}

static FlintError nativeGet(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArray(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    if(obj->dimensions == 1 && FlintJavaObject::isPrimType(obj->type)) {
        switch(obj->type.text[0]) {
            case 'B': { /* byte */
                int8_t value = ((FlintInt8Array *)obj)->getData()[index];
                FlintJavaByte *byteObj;
                FlintError err = execution.flint.newByte(value, byteObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)byteObj);
                execution.stackPushObject(byteObj);
                return ERR_OK;
            }
            case 'Z': { /* boolean */
                int8_t value = ((FlintInt8Array *)obj)->getData()[index];
                FlintJavaBoolean *boolObj;
                FlintError err = execution.flint.newBoolean(value, boolObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)boolObj);
                execution.stackPushObject(boolObj);
                return ERR_OK;
            }
            case 'C': { /* char */
                int16_t value = ((FlintInt16Array *)obj)->getData()[index];
                FlintJavaCharacter *charObj;
                FlintError err = execution.flint.newCharacter(value, charObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)charObj);
                execution.stackPushObject(charObj);
                return ERR_OK;
            }
            case 'S': { /* short */
                int16_t value = ((FlintInt16Array *)obj)->getData()[index];
                FlintJavaShort *shortObj;
                FlintError err = execution.flint.newShort(value, shortObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)shortObj);
                execution.stackPushObject(shortObj);
                return ERR_OK;
            }
            case 'I': { /* integer */
                int32_t value = ((FlintInt32Array *)obj)->getData()[index];
                FlintJavaInteger *intObj;
                FlintError err = execution.flint.newInteger(value, intObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)intObj);
                execution.stackPushObject(intObj);
                return ERR_OK;
            }
            case 'F': { /* float */
                float value = ((FlintFloatArray *)obj)->getData()[index];
                FlintJavaFloat *floatObj;
                FlintError err = execution.flint.newFloat(value, floatObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)floatObj);
                execution.stackPushObject(floatObj);
                return ERR_OK;
            }
            case 'D': { /* double */
                double value = ((FlintDoubleArray *)obj)->getData()[index];
                FlintJavaDouble *doubleObj;
                FlintError err = execution.flint.newDouble(value, doubleObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)doubleObj);
                execution.stackPushObject(doubleObj);
                return ERR_OK;
            }
            default: { /* long */
                int64_t value = ((FlintInt64Array *)obj)->getData()[index];
                FlintJavaLong *longObj;
                FlintError err = execution.flint.newLong(value, longObj);
                if(err != ERR_OK)
                    return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)longObj);
                execution.stackPushObject(longObj);
                return ERR_OK;
            }
        }
    }
    else {
        execution.stackPushObject(((FlintObjectArray *)obj)->getData()[index]);
        return ERR_OK;
    }
}

static FlintError nativeGetBoolean(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    if(obj->type.text[0] != 'Z')
        return throwIllegalArgumentException(execution, "Argument type mismatch");
    execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
    return ERR_OK;
}

static FlintError nativeGetByte(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    if(obj->type.text[0] != 'B')
        return throwIllegalArgumentException(execution, "Argument type mismatch");
    execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
    return ERR_OK;
}

static FlintError nativeGetChar(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    if(obj->type.text[0] != 'C')
        return throwIllegalArgumentException(execution, "Argument type mismatch");
    execution.stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
    return ERR_OK;
}

static FlintError nativeGetShort(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'S': /* short */
            execution.stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeGetInt(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            execution.stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            execution.stackPushInt32(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeGetLong(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            execution.stackPushInt64(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            execution.stackPushInt64(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            execution.stackPushInt64(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'J': /* long */
            execution.stackPushInt64(((FlintInt64Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeGetFloat(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            execution.stackPushFloat(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            execution.stackPushFloat(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            execution.stackPushFloat(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'F': /* float */
            execution.stackPushFloat(((FlintFloatArray *)obj)->getData()[index]);
            return ERR_OK;
        case 'J': /* long */
            execution.stackPushFloat(((FlintInt64Array *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeGetDouble(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            execution.stackPushDouble(((FlintInt8Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'C': /* char */
        case 'S': /* short */
            execution.stackPushDouble(((FlintInt16Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'I': /* integer */
            execution.stackPushDouble(((FlintInt32Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'F': /* float */
            execution.stackPushDouble(((FlintFloatArray *)obj)->getData()[index]);
            return ERR_OK;
        case 'J': /* long */
            execution.stackPushDouble(((FlintInt64Array *)obj)->getData()[index]);
            return ERR_OK;
        case 'D': /* double */
            execution.stackPushDouble(((FlintDoubleArray *)obj)->getData()[index]);
            return ERR_OK;
        default:
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSet(FlintExecution &execution) {
    FlintJavaObject *value = execution.stackPopObject();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArray(execution, obj));
    if((obj->dimensions == 1) && (FlintJavaObject::isPrimType(obj->type))) {
        if((!value) || (value->dimensions != 0))
            return throwIllegalArgumentException(execution, "Argument type mismatch");
        switch(obj->type.text[0]) {
            case 'Z': { /* boolean */
                if(value->type == *(FlintConstUtf8 *)booleanClassName) {
                    ((FlintInt8Array *)obj)->getData()[index] = (int8_t)((FlintJavaBoolean *)value)->getValue();
                    return ERR_OK;
                }
                return throwIllegalArgumentException(execution, "Argument type mismatch");
            }
            case 'B': { /* byte */
                if(value->type == *(FlintConstUtf8 *)byteClassName) {
                    ((FlintInt8Array *)obj)->getData()[index] = (int8_t)((FlintJavaByte *)value)->getValue();
                    return ERR_OK;
                }
                return throwIllegalArgumentException(execution, "Argument type mismatch");
            }
            case 'C': { /* char */
                if(value->type == *(FlintConstUtf8 *)characterClassName) {
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaCharacter *)value)->getValue();
                    return ERR_OK;
                }
                return throwIllegalArgumentException(execution, "Argument type mismatch");
            }
            case 'S': { /* short */
                if(value->type == *(FlintConstUtf8 *)byteClassName)
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaByte *)value)->getValue();
                else if(value->type == *(FlintConstUtf8 *)shortClassName)
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaShort *)value)->getValue();
                else
                    return throwIllegalArgumentException(execution, "Argument type mismatch");
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
                    return throwIllegalArgumentException(execution, "Argument type mismatch");
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
                    return throwIllegalArgumentException(execution, "Argument type mismatch");
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
                    return throwIllegalArgumentException(execution, "Argument type mismatch");
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
                    return throwIllegalArgumentException(execution, "Argument type mismatch");
                return ERR_OK;
            }
        }
    }
    else {
        RETURN_IF_ERR(checkIndex(execution, obj, index));
        ((FlintObjectArray *)obj)->getData()[index] = value;
        return ERR_OK;
    }
}

static FlintError nativeSetBoolean(FlintExecution &execution) {
    int8_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    if(obj->type.text[0] != 'Z')
        return throwIllegalArgumentException(execution, "Argument type mismatch");
    ((FlintInt8Array *)obj)->getData()[index] = !!value;
    return ERR_OK;
}

static FlintError nativeSetByte(FlintExecution &execution) {
    int8_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
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
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSetChar(FlintExecution &execution) {
    int16_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
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
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSetShort(FlintExecution &execution) {
    int16_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
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
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSetInt(FlintExecution &execution) {
    int32_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
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
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSetLong(FlintExecution &execution) {
    int64_t value = execution.stackPopInt64();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
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
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSetFloat(FlintExecution &execution) {
    float value = execution.stackPopFloat();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    switch(obj->type.text[0]) {
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return ERR_OK;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return ERR_OK;
        default:
            return throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static FlintError nativeSetDouble(FlintExecution &execution) {
    double value = execution.stackPopDouble();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    RETURN_IF_ERR(checkIsArrayOfPrimitiveType(execution, obj));
    RETURN_IF_ERR(checkIndex(execution, obj, index));
    if(obj->type.text[0] != 'D')
        return throwIllegalArgumentException(execution, "Argument type mismatch");
    ((FlintDoubleArray *)obj)->getData()[index] = value;
    return ERR_OK;
}

static FlintError nativeNewArray(FlintExecution &execution) {
    int32_t length = execution.stackPopInt32();
    FlintJavaClass *componentType = (FlintJavaClass *)execution.stackPopObject();
    RETURN_IF_ERR(checkIsClassType(execution, componentType));
    RETURN_IF_ERR(checkLength(execution, length));
    uint32_t dimensions;
    const FlintConstUtf8 &typeName = componentType->getBaseTypeName(execution.flint, &dimensions);
    if(typeName == *(FlintConstUtf8 *)voidPrimTypeName) /* void */
        return throwIllegalArgumentException(execution);
    uint8_t atype = FlintJavaObject::isPrimType(typeName);
    uint8_t typeSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
    FlintJavaObject *array;
    RETURN_IF_ERR(execution.flint.newObject(typeSize * length, typeName, dimensions + 1, array));
    memset((void *)&array->getFields(), 0, array->size);
    execution.stackPushObject(array);
    return ERR_OK;
}

static FlintError nativeMultiNewArray(FlintExecution &execution) {
    FlintInt32Array *dimensions = (FlintInt32Array *)execution.stackPopObject();
    FlintJavaClass *componentType = (FlintJavaClass *)execution.stackPopObject();
    RETURN_IF_ERR(checkIsClassType(execution, componentType));
    RETURN_IF_ERR(checkDimensions(execution, dimensions));
    uint32_t endDims;
    const FlintConstUtf8 &typeName = componentType->getBaseTypeName(execution.flint, &endDims);
    if(typeName == *(FlintConstUtf8 *)voidPrimTypeName) /* void */
        return throwIllegalArgumentException(execution);
    if((dimensions->getLength() + endDims) > 255)
        return throwIllegalArgumentException(execution);
    FlintJavaObject *array;
    RETURN_IF_ERR(execution.flint.newMultiArray(typeName, dimensions->getData(), dimensions->getLength() + endDims, endDims + 1, array));
    execution.stackPushObject(array);
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
