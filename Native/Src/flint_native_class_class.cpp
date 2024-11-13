
#include <string.h>
#include "flint.h"
#include "flint_class.h"
#include "flint_const_name.h"
#include "flint_native_class_class.h"

static void nativeGetPrimitiveClass(FlintExecution &execution) {
    FlintString *str = (FlintString *)execution.stackPopObject();
    if(str->getCoder() == 0) {
        uint32_t len = str->getLength();
        switch(len) {
            case 3:
                if(strncmp(str->getText(), "int", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("int", len));
                    return;
                }
                break;
            case 4: {
                const char *text = str->getText();
                if(strncmp(text, "void", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("void", len));
                    return;
                }
                else if(strncmp(text, "byte", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("byte", len));
                    return;
                }
                else if(strncmp(text, "char", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("char", len));
                    return;
                }
                else if(strncmp(text, "long", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("long", len));
                    return;
                }
                break;
            }
            case 5: {
                const char *text = str->getText();
                if(strncmp(text, "float", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("float", len));
                    return;
                }
                else if(strncmp(text, "short", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("short", len));
                    return;
                }
                break;
            }
            case 6:
                if(strncmp(str->getText(), "double", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("double", len));
                    return;
                }
                break;
            case 7:
                if(strncmp(str->getText(), "boolean", len) == 0) {
                    execution.stackPushObject(&execution.flint.getConstClass("boolean", len));
                    return;
                }
                break;
            default:
                break;
        }
    }
    throw "primitive type name is invalid";
}

static void nativeForName(FlintExecution &execution) {
    throw "forName is not implemented in VM";
}

static void nativeIsInstance(FlintExecution &execution) {
    FlintObject *obj = (FlintObject *)execution.stackPopObject();
    FlintClass *clsObj = (FlintClass *)execution.stackPopObject();
    FlintString &typeName = clsObj->getName();
    if(typeName.getCoder() == 0)
        execution.stackPushInt32(execution.flint.isInstanceof(obj, typeName.getText(), typeName.getLength()) ? 1 : 0);
    else
        execution.stackPushInt32(0);
}

static void nativeIsAssignableFrom(FlintExecution &execution) {
    throw "isAssignableFrom is not implemented in VM";
}

static void nativeIsInterface(FlintExecution &execution) {
    throw "isInterface is not implemented in VM";
}

static void nativeIsArray(FlintExecution &execution) {
    FlintClass *clsObj = (FlintClass *)execution.stackPopObject();
    FlintString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[')
        execution.stackPushInt32(1);
    else
        execution.stackPushInt32(0);
}

static void nativeIsPrimitive(FlintExecution &execution) {
    FlintClass *clsObj = (FlintClass *)execution.stackPopObject();
    FlintString &name = clsObj->getName();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    uint8_t result = 0;
    if(coder == 0) {
        switch(length) {
            case 3:
                if(strncmp(name.getText(), "int", length) == 0)
                    result = 1;
                break;
            case 4: {
                const char *text = name.getText();
                if(strncmp(text, "void", length) == 0)
                    result = 1;
                else if(strncmp(text, "byte", length) == 0)
                    result = 1;
                else if(strncmp(text, "char", length) == 0)
                    result = 1;
                else if(strncmp(text, "long", length) == 0)
                    result = 1;
                break;
            }
            case 5: {
                const char *text = name.getText();
                if(strncmp(text, "float", length) == 0)
                    result = 1;
                else if(strncmp(text, "short", length) == 0)
                    result = 1;
                break;
            }
            case 6:
                if(strncmp(name.getText(), "double", length) == 0)
                    result = 1;
                break;
            case 7:
                if(strncmp(name.getText(), "boolean", length) == 0)
                    result = 1;
                break;
            default:
                break;
        }
    }
    execution.stackPushInt32(result);
}

static void nativeGetSuperclass(FlintExecution &execution) {
    throw "getSuperclass is not implemented in VM";
}

static void nativeGetInterfaces(FlintExecution &execution) {
    throw "getInterfaces is not implemented in VM";
}

static void nativeGetComponentType(FlintExecution &execution) {
    FlintClass *clsObj = (FlintClass *)execution.stackPopObject();
    FlintString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[') {
        uint32_t start = (text[1] == 'L') ? 2 : 1;
        uint32_t end = (text[1] == 'L') ? (length - 1) : length;
        uint32_t len = end - start;
        FlintClass *ret;
        if(len == 1) {
            switch(text[start]) {
                case 'Z':   /* boolean */
                    ret = &execution.flint.getConstClass("boolean", 7);
                    break;
                case 'C':   /* char */
                    ret = &execution.flint.getConstClass("char", 4);
                    break;
                case 'F':   /* float */
                    ret = &execution.flint.getConstClass("float", 5);
                    break;
                case 'D':   /* double */
                    ret = &execution.flint.getConstClass("double", 6);
                    break;
                case 'B':   /* byte */
                    ret = &execution.flint.getConstClass("byte", 4);
                    break;
                case 'S':   /* short */
                    ret = &execution.flint.getConstClass("short", 5);
                    break;
                case 'I':   /* integer */
                    ret = &execution.flint.getConstClass("int", 3);
                    break;
                default:    /* long */
                    ret = &execution.flint.getConstClass("long", 4);
                    break;
            }
        }
        else
            ret = &execution.flint.getConstClass(&text[start], len);
        execution.stackPushObject(ret);
    }
    else
        execution.stackPushInt32(0);
}

static void nativeGetModifiers(FlintExecution &execution) {
    throw "getModifiers is not implemented in VM";
}

static void nativeIsHidden(FlintExecution &execution) {
    throw "isHidden is not implemented in VM";
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\x0A\x98""getPrimitiveClass", "\x25\x00\x56\xAD""(Ljava/lang/String;)Ljava/lang/Class;", nativeGetPrimitiveClass),
    NATIVE_METHOD("\x07\x00\x79\xE0""forName",           "\x25\x00\x56\xAD""(Ljava/lang/String;)Ljava/lang/Class;", nativeForName),
    NATIVE_METHOD("\x0A\x00\x4B\x35""isInstance",        "\x15\x00\x08\xBB""(Ljava/lang/Object;)Z",                 nativeIsInstance),
    NATIVE_METHOD("\x10\x00\x3C\x0E""isAssignableFrom",  "\x14\x00\x8A\x77""(Ljava/lang/Class;)Z",                  nativeIsAssignableFrom),
    NATIVE_METHOD("\x0B\x00\xA5\xE1""isInterface",       "\x03\x00\x91\x9C""()Z",                                   nativeIsInterface),
    NATIVE_METHOD("\x07\x00\x79\xE4""isArray",           "\x03\x00\x91\x9C""()Z",                                   nativeIsArray),
    NATIVE_METHOD("\x0B\x00\x21\x49""isPrimitive",       "\x03\x00\x91\x9C""()Z",                                   nativeIsPrimitive),
    NATIVE_METHOD("\x0D\x00\x38\xF1""getSuperclass",     "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetSuperclass),
    NATIVE_METHOD("\x0D\x00\xC7\x4B""getInterfaces",     "\x14\x00\xEA\x91""()[Ljava/lang/Class;",                  nativeGetInterfaces),
    NATIVE_METHOD("\x10\x00\x95\x8C""getComponentType",  "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetComponentType),
    NATIVE_METHOD("\x0C\x00\x21\x8F""getModifiers",      "\x03\x00\xD0\x51""()I",                                   nativeGetModifiers),
    NATIVE_METHOD("\x08\x00\x9C\xA3""isHidden",          "\x03\x00\x91\x9C""()Z",                                   nativeIsHidden),
};

const FlintNativeClass CLASS_CLASS = NATIVE_CLASS(classClassName, methods);
