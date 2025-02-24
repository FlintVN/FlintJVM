
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_const_name.h"
#include "flint_native_class_class.h"

static void nativeGetPrimitiveClass(FlintExecution &execution) {
    FlintJavaString *str = (FlintJavaString *)execution.stackPopObject();
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
    FlintJavaObject *obj = (FlintJavaObject *)execution.stackPopObject();
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaString &typeName = clsObj->getName();
    if(typeName.getCoder() == 0)
        execution.stackPushInt32(execution.flint.isInstanceof(obj, typeName.getText(), typeName.getLength()) ? 1 : 0);
    else
        execution.stackPushInt32(0);
}

static void nativeIsAssignableFrom(FlintExecution &execution) {
    FlintJavaClass *cls = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaClass *thisCls = (FlintJavaClass *)execution.stackPopObject();
    if(cls == NULL || thisCls == NULL)
        throw &execution.flint.newNullPointerException();
    uint32_t clsDims, thisDims;
    FlintConstUtf8 *clsTypeName = (FlintConstUtf8 *)cls->getComponentTypeName(execution.flint, &clsDims);
    FlintConstUtf8 *thisTypeName = (FlintConstUtf8 *)thisCls->getComponentTypeName(execution.flint, &thisDims);
    if(clsTypeName == NULL || thisTypeName == NULL)
        return execution.stackPushInt32(0);
    execution.stackPushInt32(execution.flint.isInstanceof(*clsTypeName, clsDims, *thisTypeName, thisDims) ? 1 : 0);
}

static void nativeIsInterface(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive())
        execution.stackPushInt32(0);
    else {
        FlintConstUtf8 *typeName = (FlintConstUtf8 *)clsObj->getComponentTypeName(execution.flint);
        if(typeName == NULL)
            return execution.stackPushInt32(0);
        int32_t modifiers = execution.flint.load(*typeName).getAccessFlag();
        execution.stackPushInt32((modifiers & 0x0200) ? 1 : 0);
    }
}

static void nativeIsArray(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    execution.stackPushInt32(clsObj->isArray());
}

static void nativeIsPrimitive(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    execution.stackPushInt32(clsObj->isPrimitive());
}

static void nativeGetSuperclass(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    int32_t modifiers;
    if(clsObj->isArray() || clsObj->isPrimitive())
        execution.stackPushObject(NULL);
    else {
        FlintConstUtf8 *typeName = (FlintConstUtf8 *)clsObj->getComponentTypeName(execution.flint);
        if(typeName == NULL)
            return execution.stackPushObject(NULL);
        FlintConstUtf8 *superClass = &execution.flint.load(*typeName).getSuperClass();
        if(superClass == NULL)
            return execution.stackPushObject(NULL);
        execution.stackPushObject(&execution.flint.getConstClass(superClass->text, superClass->length));
    }
}

static void nativeGetInterfaces0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive())
        execution.stackPushObject(&execution.flint.newObjectArray(classClassName, 0));
    else {
        FlintConstUtf8 *typeName = (FlintConstUtf8 *)clsObj->getComponentTypeName(execution.flint);
        if(typeName == NULL)
            return execution.stackPushObject(&execution.flint.newObjectArray(classClassName, 0));
        FlintClassLoader &loader = execution.flint.load(*typeName);
        uint32_t interfaceCount = loader.getInterfacesCount();
        if(interfaceCount) {
            FlintObjectArray &clsArr = execution.flint.newObjectArray(classClassName, interfaceCount);
            for(uint32_t i = 0; i < interfaceCount; i++) {
                FlintConstUtf8 &interfaceName = loader.getInterface(i);
                clsArr.getData()[i] = &execution.flint.getConstClass(interfaceName.text, interfaceName.length);
            }
            execution.stackPushObject(&clsArr);
        }
        else
            execution.stackPushObject(&execution.flint.newObjectArray(classClassName, 0));
    }
}

static void nativeGetComponentType(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[') {
        uint32_t start = (text[1] == 'L') ? 2 : 1;
        uint32_t end = (text[1] == 'L') ? (length - 1) : length;
        uint32_t len = end - start;
        FlintJavaClass *ret;
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
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    int32_t modifiers;
    if(clsObj->isArray() || clsObj->isPrimitive())
        modifiers = 0x0411;
    else {
        FlintConstUtf8 *typeName = (FlintConstUtf8 *)clsObj->getComponentTypeName(execution.flint);
        modifiers = typeName ? (execution.flint.load(*typeName).getAccessFlag() & 0xFFDF) : 0x0411;
    }
    execution.stackPushInt32(modifiers);
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
    NATIVE_METHOD("\x0E\x00\x4A\x39""getInterfaces0",    "\x14\x00\xEA\x91""()[Ljava/lang/Class;",                  nativeGetInterfaces0),
    NATIVE_METHOD("\x10\x00\x95\x8C""getComponentType",  "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetComponentType),
    NATIVE_METHOD("\x0C\x00\x21\x8F""getModifiers",      "\x03\x00\xD0\x51""()I",                                   nativeGetModifiers),
    NATIVE_METHOD("\x08\x00\x9C\xA3""isHidden",          "\x03\x00\x91\x9C""()Z",                                   nativeIsHidden),
};

const FlintNativeClass CLASS_CLASS = NATIVE_CLASS(classClassName, methods);
