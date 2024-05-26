
#include <string.h>
#include "mjvm.h"
#include "mjvm_class.h"
#include "mjvm_const_name.h"
#include "mjvm_native_class_class.h"

static bool nativeGetPrimitiveClass(Execution &execution) {
    throw "getPrimitiveClass is not implemented in VM";
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
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    uint8_t result = 0;
    if(coder == 0) {
        switch(length) {
            case 3:
                if(strncmp(text, "int", length) == 0)
                    result = 1;
                break;
            case 4:
                if(strncmp(text, "byte", length) == 0)
                    result = 1;
                else if(strncmp(text, "long", length) == 0)
                    result = 1;
                break;
            case 5:
                if(strncmp(text, "float", length) == 0)
                    result = 1;
                break;
            case 6:
                if(strncmp(text, "double", length) == 0)
                    result = 1;
                break;
            case 7:
                if(strncmp(text, "boolean", length) == 0)
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
    NATIVE_METHOD("\x11\x00""getPrimitiveClass", "\x25\x00""(Ljava/lang/String;)Ljava/lang/Class;", nativeGetPrimitiveClass),
    NATIVE_METHOD("\x07\x00""forName",           "\x25\x00""(Ljava/lang/String;)Ljava/lang/Class;", nativeForName),
    NATIVE_METHOD("\x0A\x00""isInstance",        "\x15\x00""(Ljava/lang/Object;)Z",                 nativeIsInstance),
    NATIVE_METHOD("\x10\x00""isAssignableFrom",  "\x14\x00""(Ljava/lang/Class;)Z",                  nativeIsAssignableFrom),
    NATIVE_METHOD("\x0B\x00""isInterface",       "\x03\x00""()Z",                                   nativeIsInterface),
    NATIVE_METHOD("\x07\x00""isArray",           "\x03\x00""()Z",                                   nativeIsArray),
    NATIVE_METHOD("\x0B\x00""isPrimitive",       "\x03\x00""()Z",                                   nativeIsPrimitive),
    NATIVE_METHOD("\x0D\x00""getSuperclass",     "\x13\x00""()Ljava/lang/Class;",                   nativeGetSuperclass),
    NATIVE_METHOD("\x0D\x00""getInterfaces",     "\x14\x00""()[Ljava/lang/Class;",                  nativeGetInterfaces),
    NATIVE_METHOD("\x10\x00""getComponentType",  "\x13\x00""()Ljava/lang/Class;",                   nativeGetComponentType),
    NATIVE_METHOD("\x0C\x00""getModifiers",      "\x03\x00""()I",                                   nativeGetModifiers),
    NATIVE_METHOD("\x08\x00""isHidden",          "\x03\x00""()Z",                                   nativeIsHidden),
};

const NativeClass CLASS_CLASS = NATIVE_CLASS(classClassName, methods);
