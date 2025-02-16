
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_const_name.h"
#include "flint_native_array_class.h"
#include "flint_java_boolean.h"
#include "flint_java_byte.h"
#include "flint_java_character.h"
#include "flint_java_short.h"
#include "flint_java_integer.h"
#include "flint_java_float.h"
#include "flint_java_long.h"
#include "flint_java_double.h"

static void throwIllegalArgumentException(FlintExecution &execution, const char *msg) {
    if(msg) {
        int32_t len = strlen(msg);
        FlintJavaString *strObj = &execution.flint.newString(msg, len);
        throw &execution.flint.newIllegalArgumentException(strObj);
    }
    else
        throw &execution.flint.newIllegalArgumentException();
}

static void checkIsArray(FlintExecution &execution, FlintJavaObject *obj) {
    if(obj == NULL)
        throw &execution.flint.newNullPointerException();
    if(obj->dimensions < 1)
        throwIllegalArgumentException(execution, "Argument is not an array");
}

static void checkIsClassType(FlintExecution &execution, FlintJavaObject *obj) {
    if(obj == NULL)
        throw &execution.flint.newNullPointerException();
    else if(obj->type != classClassName)
        throw &execution.flint.newIllegalArgumentException();
}

static void checkIndex(FlintExecution &execution, FlintJavaObject *obj, int32_t index) {
    int32_t length = obj->size / obj->parseTypeSize();
    if((index < 0) || (index >= length))
        throw &execution.flint.newArrayIndexOutOfBoundsException();
}

static void checkLength(FlintExecution &execution, int32_t size) {
    if(size < 0)
        throw &execution.flint.newNegativeArraySizeException();
}

static void checkDimensions(FlintExecution &execution, FlintInt32Array *dimensions) {
    if(
        (dimensions == NULL) ||
        (dimensions->dimensions != 1) ||
        (dimensions->type != *primTypeConstUtf8List[6]) ||
        (dimensions->getLength() == 0) ||
        (dimensions->getLength() > 255)
    ) {
        throw &execution.flint.newIllegalArgumentException();
    }
}

static void checkIsArrayOfPrimitiveType(FlintExecution &execution, FlintJavaObject *obj) {
    checkIsArray(execution, obj);
    if((obj->dimensions != 1) || !FlintJavaObject::isPrimType(obj->type))
        throwIllegalArgumentException(execution, "Argument is not an array of primitive type");
}

static void nativeGetLength(FlintExecution &execution) {
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArray(execution, obj);
    execution.stackPushInt32(obj->size / obj->parseTypeSize());
}

static void nativeGet(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArray(execution, obj);
    checkIndex(execution, obj, index);
    if(obj->dimensions == 1 && FlintJavaObject::isPrimType(obj->type)) {
        switch(obj->type.text[0]) {
            case 'B': { /* byte */
                int8_t value = ((FlintInt8Array *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newByte(value));
            }
            case 'Z': { /* boolean */
                int8_t value = ((FlintInt8Array *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newBoolean(value));
            }
            case 'C': { /* char */
                int16_t value = ((FlintInt16Array *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newCharacter(value));
            }
            case 'S': { /* short */
                int16_t value = ((FlintInt16Array *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newShort(value));
            }
            case 'I': { /* integer */
                int32_t value = ((FlintInt32Array *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newInteger(value));
            }
            case 'F': { /* float */
                float value = ((FlintFloatArray *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newFloat(value));
            }
            case 'D': { /* double */
                double value = ((FlintDoubleArray *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newDouble(value));
            }
            default: { /* long */
                int64_t value = ((FlintInt64Array *)obj)->getData()[index];
                return execution.stackPushObject(&execution.flint.newLong(value));
            }
        }
    }
    else
        execution.stackPushObject(((FlintObjectArray *)obj)->getData()[index]);
}

static void nativeGetBoolean(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    if(obj->type.text[0] == 'Z')
        execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
    else
        throwIllegalArgumentException(execution, "Argument type mismatch");
}

static void nativeGetByte(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    if(obj->type.text[0] == 'B')
        execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
    else
        throwIllegalArgumentException(execution, "Argument type mismatch");
}

static void nativeGetChar(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    if(obj->type.text[0] == 'C')
        execution.stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
    else
        throwIllegalArgumentException(execution, "Argument type mismatch");
}

static void nativeGetShort(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            return execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
        case 'S': /* short */
            return execution.stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeGetInt(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            return execution.stackPushInt32(((FlintInt8Array *)obj)->getData()[index]);
        case 'C': /* char */
        case 'S': /* short */
            return execution.stackPushInt32(((FlintInt16Array *)obj)->getData()[index]);
        case 'I': /* integer */
            return execution.stackPushInt32(((FlintInt32Array *)obj)->getData()[index]);
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeGetLong(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            return execution.stackPushInt64(((FlintInt8Array *)obj)->getData()[index]);
        case 'C': /* char */
        case 'S': /* short */
            return execution.stackPushInt64(((FlintInt16Array *)obj)->getData()[index]);
        case 'I': /* integer */
            return execution.stackPushInt64(((FlintInt32Array *)obj)->getData()[index]);
        case 'J': /* long */
            return execution.stackPushInt64(((FlintInt64Array *)obj)->getData()[index]);
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeGetFloat(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            return execution.stackPushFloat(((FlintInt8Array *)obj)->getData()[index]);
        case 'C': /* char */
        case 'S': /* short */
            return execution.stackPushFloat(((FlintInt16Array *)obj)->getData()[index]);
        case 'I': /* integer */
            return execution.stackPushFloat(((FlintInt32Array *)obj)->getData()[index]);
        case 'F': /* float */
            return execution.stackPushFloat(((FlintFloatArray *)obj)->getData()[index]);
        case 'J': /* long */
            return execution.stackPushFloat(((FlintInt64Array *)obj)->getData()[index]);
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeGetDouble(FlintExecution &execution) {
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            return execution.stackPushDouble(((FlintInt8Array *)obj)->getData()[index]);
        case 'C': /* char */
        case 'S': /* short */
            return execution.stackPushDouble(((FlintInt16Array *)obj)->getData()[index]);
        case 'I': /* integer */
            return execution.stackPushDouble(((FlintInt32Array *)obj)->getData()[index]);
        case 'F': /* float */
            return execution.stackPushDouble(((FlintFloatArray *)obj)->getData()[index]);
        case 'J': /* long */
            return execution.stackPushDouble(((FlintInt64Array *)obj)->getData()[index]);
        case 'D': /* double */
            return execution.stackPushDouble(((FlintDoubleArray *)obj)->getData()[index]);
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSet(FlintExecution &execution) {
    FlintJavaObject *value = execution.stackPopObject();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArray(execution, obj);
    if((obj->dimensions == 1) && (FlintJavaObject::isPrimType(obj->type))) {
        if((!value) || (value->dimensions != 0))
            throwIllegalArgumentException(execution, "Argument type mismatch");
        switch(obj->type.text[0]) {
            case 'Z': { /* boolean */
                if(value->type == booleanClassName) {
                    ((FlintInt8Array *)obj)->getData()[index] = (int8_t)((FlintJavaBoolean *)value)->getValue();
                    return;
                }
                throwIllegalArgumentException(execution, "Argument type mismatch");
            }
            case 'B': { /* byte */
                if(value->type == byteClassName) {
                    ((FlintInt8Array *)obj)->getData()[index] = (int8_t)((FlintJavaByte *)value)->getValue();
                    return;
                }
                throwIllegalArgumentException(execution, "Argument type mismatch");
            }
            case 'C': { /* char */
                if(value->type == characterClassName) {
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaCharacter *)value)->getValue();
                    return;
                }
                throwIllegalArgumentException(execution, "Argument type mismatch");
            }
            case 'S': { /* short */
                if(value->type == byteClassName)
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaByte *)value)->getValue();
                else if(value->type == shortClassName)
                    ((FlintInt16Array *)obj)->getData()[index] = (int8_t)((FlintJavaShort *)value)->getValue();
                else
                    throwIllegalArgumentException(execution, "Argument type mismatch");
                return;
            }
            case 'I': { /* integer */
                if(value->type == byteClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == characterClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == shortClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == integerClassName)
                    ((FlintInt32Array *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else
                    throwIllegalArgumentException(execution, "Argument type mismatch");
                return;
            }
            case 'F': { /* float */
                if(value->type == byteClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == characterClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == shortClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == integerClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else if(value->type == floatClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaFloat *)value)->getValue();
                else if(value->type == longClassName)
                    ((FlintFloatArray *)obj)->getData()[index] = ((FlintJavaLong *)value)->getValue();
                else
                    throwIllegalArgumentException(execution, "Argument type mismatch");
                return;
            }
            case 'D': { /* double */
                if(value->type == byteClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == characterClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == shortClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == integerClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else if(value->type == floatClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaFloat *)value)->getValue();
                else if(value->type == longClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaLong *)value)->getValue();
                else if(value->type == doubleClassName)
                    ((FlintDoubleArray *)obj)->getData()[index] = ((FlintJavaDouble *)value)->getValue();
                else
                    throwIllegalArgumentException(execution, "Argument type mismatch");
                return;
            }
            default: { /* long */
                if(value->type == byteClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaByte *)value)->getValue();
                else if(value->type == characterClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaCharacter *)value)->getValue();
                else if(value->type == shortClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaShort *)value)->getValue();
                else if(value->type == integerClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaInteger *)value)->getValue();
                else if(value->type == longClassName)
                    ((FlintInt64Array *)obj)->getData()[index] = ((FlintJavaLong *)value)->getValue();
                else
                    throwIllegalArgumentException(execution, "Argument type mismatch");
                return;
            }
        }
    }
    else {
        checkIndex(execution, obj, index);
        ((FlintObjectArray *)obj)->getData()[index] = value;
    }
}

static void nativeSetBoolean(FlintExecution &execution) {
    int8_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    if(obj->type.text[0] == 'Z')
        ((FlintInt8Array *)obj)->getData()[index] = !!value;
    else
        throwIllegalArgumentException(execution, "Argument type mismatch");
}

static void nativeSetByte(FlintExecution &execution) {
    int8_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'B': /* byte */
            ((FlintInt8Array *)obj)->getData()[index] = value;
            return;
        case 'S': /* short */
            ((FlintInt16Array *)obj)->getData()[index] = value;
            return;
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return;
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSetChar(FlintExecution &execution) {
    int16_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'C': /* char */
            ((FlintInt16Array *)obj)->getData()[index] = value;
            return;
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return;
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSetShort(FlintExecution &execution) {
    int16_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'S': /* short */
            ((FlintInt16Array *)obj)->getData()[index] = value;
            return;
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return;
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSetInt(FlintExecution &execution) {
    int32_t value = execution.stackPopInt32();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'I': /* integer */
            ((FlintInt32Array *)obj)->getData()[index] = value;
            return;
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return;
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSetLong(FlintExecution &execution) {
    int64_t value = execution.stackPopInt64();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return;
        case 'J': /* long */
            ((FlintInt64Array *)obj)->getData()[index] = value;
            return;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return;
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSetFloat(FlintExecution &execution) {
    float value = execution.stackPopFloat();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    switch(obj->type.text[0]) {
        case 'F': /* float */
            ((FlintFloatArray *)obj)->getData()[index] = value;
            return;
        case 'D': /* double */
            ((FlintDoubleArray *)obj)->getData()[index] = value;
            return;
        default:
            throwIllegalArgumentException(execution, "Argument type mismatch");
    }
}

static void nativeSetDouble(FlintExecution &execution) {
    double value = execution.stackPopDouble();
    int32_t index = execution.stackPopInt32();
    FlintJavaObject *obj = execution.stackPopObject();
    checkIsArrayOfPrimitiveType(execution, obj);
    checkIndex(execution, obj, index);
    if(obj->type.text[0] == 'D')
        ((FlintDoubleArray *)obj)->getData()[index] = value;
    else
        throwIllegalArgumentException(execution, "Argument type mismatch");
}

static FlintConstUtf8 &getTypeInfo(FlintExecution &execution, FlintJavaClass *cls, uint32_t *dimensions) {
    uint32_t dims = 0;
    FlintJavaString &name = cls->getName();
    const char *typeText = name.getText();
    uint32_t typeLength = name.getLength();
    while((*typeText == '[') && typeLength) {
        typeText++;
        typeLength--;
        dims++;
    }
    const FlintConstUtf8 *type = NULL;
    if(dims == 0) {
        switch(typeLength) {
            case 3:
                if(strncmp(typeText, "int", typeLength) == 0)
                    type = primTypeConstUtf8List[6];
                else
                    throwIllegalArgumentException(execution, NULL);
                break;
            case 4: {
                if(strncmp(typeText, "byte", typeLength) == 0)
                    type = primTypeConstUtf8List[4];
                else if(strncmp(typeText, "char", typeLength) == 0)
                    type = primTypeConstUtf8List[1];
                else if(strncmp(typeText, "long", typeLength) == 0)
                    type = primTypeConstUtf8List[7];
                else
                    throwIllegalArgumentException(execution, NULL);
                break;
            }
            case 5: {
                if(strncmp(typeText, "float", typeLength) == 0)
                    type = primTypeConstUtf8List[2];
                else if(strncmp(typeText, "short", typeLength) == 0)
                    type = primTypeConstUtf8List[5];
                else
                    throwIllegalArgumentException(execution, NULL);
                break;
            }
            case 6:
                if(strncmp(typeText, "double", typeLength) == 0)
                    type = primTypeConstUtf8List[3];
                else
                    throwIllegalArgumentException(execution, NULL);
                break;
            case 7:
                if(strncmp(typeText, "boolean", typeLength) == 0)
                    type = primTypeConstUtf8List[0];
                else
                    throwIllegalArgumentException(execution, NULL);
                break;
            default:
                break;
        }
    }
    else if(*typeText == 'L') {
        typeText++;
        typeLength -= 2;
    }
    if(type == NULL) {
        char *text = (char *)Flint::malloc(typeLength);
        for(uint32_t i = 0; i < typeLength; i++)
            text[i] = (typeText[i] == '.') ? '/' : typeText[i];
        try {
            type = &execution.flint.getConstUtf8(text, typeLength);
        }
        catch(...) {
            Flint::free(text);
            throw;
        }
        Flint::free(text);
    }
    if(dimensions)
        *dimensions = dims;
    return *(FlintConstUtf8 *)type;
}

static void nativeNewArray(FlintExecution &execution) {
    int32_t length = execution.stackPopInt32();
    FlintJavaClass *componentType = (FlintJavaClass *)execution.stackPopObject();
    checkIsClassType(execution, componentType);
    checkLength(execution, length);
    uint32_t dimensions;
    FlintConstUtf8 &type = getTypeInfo(execution, componentType, &dimensions);
    uint8_t atype = FlintJavaObject::isPrimType(type);
    uint8_t typeSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
    FlintJavaObject &array = execution.flint.newObject(typeSize * length, type, dimensions + 1);
    memset(&array.getFields(), 0, array.size);
    execution.stackPushObject(&array);
}

static void nativeMultiNewArray(FlintExecution &execution) {
    FlintInt32Array *dimensions = (FlintInt32Array *)execution.stackPopObject();
    FlintJavaClass *componentType = (FlintJavaClass *)execution.stackPopObject();
    checkIsClassType(execution, componentType);
    checkDimensions(execution, dimensions);
    uint32_t endDims;
    FlintConstUtf8 &type = getTypeInfo(execution, componentType, &endDims);
    if((dimensions->getLength() + endDims) > 255)
        throw &execution.flint.newIllegalArgumentException();
    FlintJavaObject &array = execution.flint.newMultiArray(type, dimensions->getData(), dimensions->getLength() + endDims, endDims + 1);
    execution.stackPushObject(&array);
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
