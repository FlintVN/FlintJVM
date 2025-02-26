
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
    // TODO
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
        FlintClassAccessFlag modifiers = execution.flint.load(*typeName).getAccessFlag();
        execution.stackPushInt32((modifiers & CLASS_INTERFACE) ? 1 : 0);
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

static FlintJavaClass &getClass(FlintExecution &execution, const char *typeName, uint16_t length) {
    if(length == 1) {
        switch(*typeName) {
            case 'Z':   /* boolean */
                return execution.flint.getConstClass("boolean", 7);
            case 'C':   /* char */
                return execution.flint.getConstClass("char", 4);
            case 'F':   /* float */
                return execution.flint.getConstClass("float", 5);
            case 'D':   /* double */
                return execution.flint.getConstClass("double", 6);
            case 'B':   /* byte */
                return execution.flint.getConstClass("byte", 4);
            case 'S':   /* short */
                return execution.flint.getConstClass("short", 5);
                break;
            case 'I':   /* integer */
                return execution.flint.getConstClass("int", 3);
            default:    /* long */
                return execution.flint.getConstClass("long", 4);
        }
    }
    else
        return execution.flint.getConstClass(typeName, length);
}

static void nativeGetComponentType(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[')
        execution.stackPushObject(&getClass(execution, &text[1], length - 1));
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

static FlintJavaClass &getReturnType(FlintExecution &execution, FlintMethodInfo &methodInfo) {
    const char *text = methodInfo.descriptor.text;
    while(*text++ != ')');
    uint16_t len = methodInfo.descriptor.length - (text - methodInfo.descriptor.text);
    return getClass(execution, text, len);
}

static uint8_t getParameterCount(FlintMethodInfo &methodInfo) {
    uint8_t count = 0;
    const char *text = methodInfo.descriptor.text;
    if(*text != '(')
        throw "the descriptor is not a description of the method";
    text++;
    while(*text) {
        if(*text == ')')
            return count;
        else if(*text == '[')
            text++;
        else {
            count++;
            if(*text++ == 'L') {
                while(*text) {
                    if(*text == ')')
                        return count;
                    else if(*text == ';') {
                        text++;
                        break;
                    }
                    text++;
                }
            }
        }
    }
    throw "descriptor is invalid";
}

static FlintObjectArray &getParameterTypes(FlintExecution &execution, FlintMethodInfo &methodInfo, FlintObjectArray &classArray0) {
    uint8_t count = getParameterCount(methodInfo);
    if(count == 0)
        return classArray0;
    FlintObjectArray &array = execution.flint.newObjectArray(classClassName, count);
    FlintJavaObject **data = array.getData();
    count = 0;
    const char *text = methodInfo.descriptor.text;
    while(*text == '(')
        text++;
    while(*text) {
        if(*text == ')')
            break;
        const char *start = text;
        while(*text == '[')
            text++;
        if(*text == 'L') {
            while(*text) {
                if(*text == ')')
                    break;
                else if(*text == ';') {
                    text++;
                    break;
                }
                text++;
            }
        }
        else
            text++;
        uint16_t len = text - start;
        data[count++] = &getClass(execution, start, len);
    }
    return array;
}

static FlintObjectArray &getExceptionTypes(FlintExecution &execution, FlintMethodInfo &methodInfo, FlintObjectArray &classArray0) {
    if(!methodInfo.hasAttributeCode())
        return classArray0;
    FlintCodeAttribute &attributeCode = methodInfo.getAttributeCode();
    if(attributeCode.exceptionTableLength == 0)
        return classArray0;
    FlintObjectArray &array = execution.flint.newObjectArray(classClassName, attributeCode.exceptionTableLength);
    FlintClassLoader &classLoader = methodInfo.classLoader;
    FlintJavaObject **data = array.getData();
    for(uint16_t i = 0; i < attributeCode.exceptionTableLength; i++) {
        FlintConstUtf8 &catchType = classLoader.getConstUtf8Class(attributeCode.getException(i).catchType);
        data[i] = &execution.flint.getConstClass(catchType.text, catchType.length);
    }
    return array;
}

static void nativeGetDeclaredMethods0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintClassLoader &loader = (clsObj->isArray() || clsObj->isPrimitive()) ? execution.flint.load(objectClassName) : execution.flint.load(*clsObj->getComponentTypeName(execution.flint));
    uint16_t methodCount = loader.getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo &methodInfo = loader.getMethodInfo(i);
        if(methodInfo.name != constructorName && methodInfo.name != staticConstructorName)
            count++;
    }
    FlintObjectArray *array = (FlintObjectArray *)&execution.flint.newObjectArray(methodClassName, count);
    count = 0;
    uint32_t clazzIndex = 0, nameIndex = 0, returnTypeIndex = 0, parameterTypesIndex = 0, exceptionTypesIndex = 0, modifiersIndex = 0;
    FlintObjectArray &classArray0 = execution.flint.getClassArray0();
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo &methodInfo = loader.getMethodInfo(i);
        if(methodInfo.name == constructorName || methodInfo.name == staticConstructorName)
            continue;

        FlintJavaObject &method = execution.flint.newObject(methodClassName);
        FlintFieldsData &fields = method.getFields();

        fields.getFieldObject(clazzFieldName, &clazzIndex).object = clsObj;
        fields.getFieldObject(nameFieldName, &nameIndex).object = &execution.flint.getConstString(methodInfo.name);
        fields.getFieldObject(returnTypeFieldName, &returnTypeIndex).object = &getReturnType(execution, methodInfo);
        fields.getFieldObject(parameterTypesFieldName, &parameterTypesIndex).object = &getParameterTypes(execution, methodInfo, classArray0);
        fields.getFieldObject(exceptionTypesFieldName, &exceptionTypesIndex).object = &getExceptionTypes(execution, methodInfo, classArray0);
        fields.getFieldData32(modifiersFieldName, &modifiersIndex).value = loader.getAccessFlag();

        array->getData()[count++] = &method;
    }
    execution.stackPushObject(array);
}

static void nativeGetDeclaredConstructors0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintClassLoader &loader = (clsObj->isArray() || clsObj->isPrimitive()) ? execution.flint.load(objectClassName) : execution.flint.load(*clsObj->getComponentTypeName(execution.flint));
    uint16_t methodCount = loader.getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo &methodInfo = loader.getMethodInfo(i);
        if(methodInfo.name == constructorName)
            count++;
    }
    FlintObjectArray *array = (FlintObjectArray *)&execution.flint.newObjectArray(constructorClassName, count);
    count = 0;
    uint32_t clazzIndex = 0, parameterTypesIndex = 0, exceptionTypesIndex = 0, modifiersIndex = 0;
    FlintObjectArray &classArray0 = execution.flint.getClassArray0();
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo &methodInfo = loader.getMethodInfo(i);
        if(methodInfo.name != constructorName)
            continue;

        FlintJavaObject &constructor = execution.flint.newObject(constructorClassName);
        FlintFieldsData &fields = constructor.getFields();

        fields.getFieldObject(clazzFieldName, &clazzIndex).object = clsObj;
        fields.getFieldObject(parameterTypesFieldName, &parameterTypesIndex).object = &getParameterTypes(execution, methodInfo, classArray0);
        fields.getFieldObject(exceptionTypesFieldName, &exceptionTypesIndex).object = &getExceptionTypes(execution, methodInfo, classArray0);
        fields.getFieldData32(modifiersFieldName, &modifiersIndex).value = loader.getAccessFlag();

        array->getData()[count++] = &constructor;
    }
    execution.stackPushObject(array);
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x11\x00\x0A\x98""getPrimitiveClass",        "\x25\x00\x56\xAD""(Ljava/lang/String;)Ljava/lang/Class;", nativeGetPrimitiveClass),
    NATIVE_METHOD("\x07\x00\x79\xE0""forName",                  "\x25\x00\x56\xAD""(Ljava/lang/String;)Ljava/lang/Class;", nativeForName),
    NATIVE_METHOD("\x0A\x00\x4B\x35""isInstance",               "\x15\x00\x08\xBB""(Ljava/lang/Object;)Z",                 nativeIsInstance),
    NATIVE_METHOD("\x10\x00\x3C\x0E""isAssignableFrom",         "\x14\x00\x8A\x77""(Ljava/lang/Class;)Z",                  nativeIsAssignableFrom),
    NATIVE_METHOD("\x0B\x00\xA5\xE1""isInterface",              "\x03\x00\x91\x9C""()Z",                                   nativeIsInterface),
    NATIVE_METHOD("\x07\x00\x79\xE4""isArray",                  "\x03\x00\x91\x9C""()Z",                                   nativeIsArray),
    NATIVE_METHOD("\x0B\x00\x21\x49""isPrimitive",              "\x03\x00\x91\x9C""()Z",                                   nativeIsPrimitive),
    NATIVE_METHOD("\x0D\x00\x38\xF1""getSuperclass",            "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetSuperclass),
    NATIVE_METHOD("\x0E\x00\x4A\x39""getInterfaces0",           "\x14\x00\xEA\x91""()[Ljava/lang/Class;",                  nativeGetInterfaces0),
    NATIVE_METHOD("\x10\x00\x95\x8C""getComponentType",         "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetComponentType),
    NATIVE_METHOD("\x0C\x00\x21\x8F""getModifiers",             "\x03\x00\xD0\x51""()I",                                   nativeGetModifiers),
    NATIVE_METHOD("\x08\x00\x9C\xA3""isHidden",                 "\x03\x00\x91\x9C""()Z",                                   nativeIsHidden),
    NATIVE_METHOD("\x13\x00\x4B\x12""getDeclaredMethods0",      "\x1D\x00\x12\x57""()[Ljava/lang/reflect/Method;",         nativeGetDeclaredMethods0),
    NATIVE_METHOD("\x18\x00\x0C\xE2""getDeclaredConstructors0", "\x23\x00\x73\x02""()[Ljava/lang/reflect/Constructors;",   nativeGetDeclaredConstructors0),
};

const FlintNativeClass CLASS_CLASS = NATIVE_CLASS(classClassName, methods);
