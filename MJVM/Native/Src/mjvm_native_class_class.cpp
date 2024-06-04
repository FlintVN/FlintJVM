
#include <string.h>
#include "mjvm.h"
#include "mjvm_class.h"
#include "mjvm_const_name.h"
#include "mjvm_native_class_class.h"

static bool nativeGetPrimitiveClass(Execution &execution) {
    MjvmString *str = (MjvmString *)execution.stackPopObject();
    if(str->getCoder() == 0) {
        uint32_t len = str->getLength();
        switch(len) {
            case 3:
                if(strncmp(str->getText(), "int", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("int", len));
                    return true;
                }
                break;
            case 4: {
                const char *text = str->getText();
                if(strncmp(text, "void", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("void", len));
                    return true;
                }
                else if(strncmp(text, "byte", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("byte", len));
                    return true;
                }
                else if(strncmp(text, "char", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("char", len));
                    return true;
                }
                else if(strncmp(text, "long", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("long", len));
                    return true;
                }
                break;
            }
            case 5: {
                const char *text = str->getText();
                if(strncmp(text, "float", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("float", len));
                    return true;
                }
                else if(strncmp(text, "short", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("short", len));
                    return true;
                }
                break;
            }
            case 6:
                if(strncmp(str->getText(), "double", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("double", len));
                    return true;
                }
                break;
            case 7:
                if(strncmp(str->getText(), "boolean", len) == 0) {
                    execution.stackPushObject(execution.getConstClass("boolean", len));
                    return true;
                }
                break;
            default:
                break;
        }
    }
    throw "primitive type name is invalid";
}

static bool nativeForName(Execution &execution) {
    throw "forName is not implemented in VM";
}

static bool nativeIsInstance(Execution &execution) {
    MjvmObject *obj = (MjvmObject *)execution.stackPopObject();
    MjvmClass *clsObj = (MjvmClass *)execution.stackPopObject();
    MjvmString &typeName = clsObj->getName();
    if(typeName.getCoder() == 0)
        execution.stackPushInt32(execution.isInstanceof(obj, typeName.getText(), typeName.getLength()) ? 1 : 0);
    else
        execution.stackPushInt32(0);
    return true;
}

static bool nativeIsAssignableFrom(Execution &execution) {
    throw "isAssignableFrom is not implemented in VM";
}

static bool nativeIsInterface(Execution &execution) {
    throw "isInterface is not implemented in VM";
}

static bool nativeIsArray(Execution &execution) {
    MjvmClass *clsObj = (MjvmClass *)execution.stackPopObject();
    MjvmString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[')
        execution.stackPushInt32(1);
    else
        execution.stackPushInt32(0);
    return true;
}

static bool nativeIsPrimitive(Execution &execution) {
    MjvmClass *clsObj = (MjvmClass *)execution.stackPopObject();
    MjvmString &name = clsObj->getName();
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
    return true;
}

static bool nativeGetSuperclass(Execution &execution) {
    throw "getSuperclass is not implemented in VM";
}

static bool nativeGetInterfaces(Execution &execution) {
    throw "getInterfaces is not implemented in VM";
}

static bool nativeGetComponentType(Execution &execution) {
    MjvmClass *clsObj = (MjvmClass *)execution.stackPopObject();
    MjvmString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[') {
        uint32_t start = (text[1] == 'L') ? 2 : 1;
        uint32_t end = (text[1] == 'L') ? (length - 1) : length;
        uint32_t len = end - start;
        MjvmClass *ret;
        if(len == 1) {
            switch(text[start]) {
                case 'Z':   /* boolean */
                    ret = execution.getConstClass("boolean", 7);
                    break;
                case 'C':   /* char */
                    ret = execution.getConstClass("char", 4);
                    break;
                case 'F':   /* float */
                    ret = execution.getConstClass("float", 5);
                    break;
                case 'D':   /* double */
                    ret = execution.getConstClass("double", 6);
                    break;
                case 'B':   /* byte */
                    ret = execution.getConstClass("byte", 4);
                    break;
                case 'S':   /* short */
                    ret = execution.getConstClass("short", 5);
                    break;
                case 'I':   /* integer */
                    ret = execution.getConstClass("int", 3);
                    break;
                default:    /* long */
                    ret = execution.getConstClass("long", 4);
                    break;
            }
        }
        else
            ret = execution.getConstClass(&text[start], len);
        execution.stackPushObject(ret);
        return true;
    }
    execution.stackPushInt32(0);
    return true;
}

static bool nativeGetModifiers(Execution &execution) {
    throw "getModifiers is not implemented in VM";
}

static bool nativeIsHidden(Execution &execution) {
    throw "isHidden is not implemented in VM";
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\xEF\x06""getPrimitiveClass", "\x25\x00\x10\x0D""(Ljava/lang/String;)Ljava/lang/Class;", nativeGetPrimitiveClass),
    NATIVE_METHOD("\x07\x00\xC8\x02""forName",           "\x25\x00\x10\x0D""(Ljava/lang/String;)Ljava/lang/Class;", nativeForName),
    NATIVE_METHOD("\x0A\x00\x11\x04""isInstance",        "\x15\x00\x2B\x07""(Ljava/lang/Object;)Z",                 nativeIsInstance),
    NATIVE_METHOD("\x10\x00\x69\x06""isAssignableFrom",  "\x14\x00\xCA\x06""(Ljava/lang/Class;)Z",                  nativeIsAssignableFrom),
    NATIVE_METHOD("\x0B\x00\x6D\x04""isInterface",       "\x03\x00\xAB\x00""()Z",                                   nativeIsInterface),
    NATIVE_METHOD("\x07\x00\xDB\x02""isArray",           "\x03\x00\xAB\x00""()Z",                                   nativeIsArray),
    NATIVE_METHOD("\x0B\x00\x95\x04""isPrimitive",       "\x03\x00\xAB\x00""()Z",                                   nativeIsPrimitive),
    NATIVE_METHOD("\x0D\x00\x65\x05""getSuperclass",     "\x13\x00\x70\x06""()Ljava/lang/Class;",                   nativeGetSuperclass),
    NATIVE_METHOD("\x0D\x00\x44\x05""getInterfaces",     "\x14\x00\xCB\x06""()[Ljava/lang/Class;",                  nativeGetInterfaces),
    NATIVE_METHOD("\x10\x00\x95\x06""getComponentType",  "\x13\x00\x70\x06""()Ljava/lang/Class;",                   nativeGetComponentType),
    NATIVE_METHOD("\x0C\x00\xE2\x04""getModifiers",      "\x03\x00\x9A\x00""()I",                                   nativeGetModifiers),
    NATIVE_METHOD("\x08\x00\x28\x03""isHidden",          "\x03\x00\xAB\x00""()Z",                                   nativeIsHidden),
};

const NativeClass CLASS_CLASS = NATIVE_CLASS(classClassName, methods);
