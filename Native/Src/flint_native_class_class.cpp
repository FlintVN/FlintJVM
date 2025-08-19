
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_const_name_base.h"
#include "flint_native_class_class.h"
#include "flint_throw_support.h"

static void freeObjectArray(Flint &flint, FlintObjectArray *array, uint32_t length) {
    for(uint32_t i = 0; i < length; i++)
        flint.freeObject(*array->getData()[i]);
    flint.freeObject(*array);
}

static FlintError checkClassIsExist(const char *type, uint32_t typeLen) {
    char buff[FILE_NAME_BUFF_SIZE];
    if((typeLen + sizeof(".class")) > FILE_NAME_BUFF_SIZE)
        return ERR_VM_ERROR;
    for(uint32_t i = 0; i < typeLen; i++) {
        if(type[i] != '/' && type[i] != '\\')
            buff[i] = (type[i] == '.') ? '/' : type[i];
        else
            return ERR_CLASS_NOT_FOUND;
    }
    memcpy(&buff[typeLen], ".class", sizeof(".class"));
    if(FlintAPI::IO::finfo(buff, NULL, NULL) != FILE_RESULT_OK)
        return ERR_CLASS_NOT_FOUND;
    return ERR_OK;
}

static FlintError loadClassForName(FlintExecution &execution, const char *type, uint32_t typeLen, FlintJavaClass *&cls) {
    RETURN_IF_ERR(checkClassIsExist(type, typeLen));
    RETURN_IF_ERR(execution.flint.getConstClass(type, typeLen, cls));
    return ERR_OK;
}

static FlintError nativeGetPrimitiveClass(FlintExecution &execution) {
    FlintJavaString *str = (FlintJavaString *)execution.stackPopObject();
    if(str->getCoder() == 0) {
        uint32_t len = str->getLength();
        bool isPrim = false;
        switch(len) {
            case 3:
                if(strncmp(str->getText(), "int", len) == 0)
                    isPrim = true;
                break;
            case 4: {
                const char *text = str->getText();
                if(strncmp(text, "void", len) == 0)
                    isPrim = true;
                else if(strncmp(text, "byte", len) == 0)
                    isPrim = true;
                else if(strncmp(text, "char", len) == 0)
                    isPrim = true;
                else if(strncmp(text, "long", len) == 0)
                    isPrim = true;
                break;
            }
            case 5: {
                const char *text = str->getText();
                if(strncmp(text, "float", len) == 0)
                    isPrim = true;
                else if(strncmp(text, "short", len) == 0)
                    isPrim = true;
                break;
            }
            case 6:
                if(strncmp(str->getText(), "double", len) == 0)
                    isPrim = true;
                break;
            case 7:
                if(strncmp(str->getText(), "boolean", len) == 0)
                    isPrim = true;
                break;
            default:
                break;
        }
        if(isPrim) {
            FlintJavaClass *cls;
            RETURN_IF_ERR(execution.flint.getConstClass(str->getText(), len, cls));
            execution.stackPushObject(cls);
            return ERR_OK;
        }
    }
    return throwIllegalArgumentException(execution, "primitive type name is invalid");
}

static FlintError nativeForName(FlintExecution &execution) {
    FlintJavaString *type = (FlintJavaString *)execution.stackPopObject();
    if(type == NULL)
        return throwNullPointerException(execution);
    FlintJavaClass *cls;
    FlintError err = loadClassForName(execution, type->getText(), type->getLength(), cls);
    if(err == ERR_OK) {
        execution.stackPushObject(cls);
        return ERR_OK;
    }
    else if(err == ERR_CLASS_NOT_FOUND)
        return throwClassNotFoundException(execution, type);
    else if(err == ERR_VM_ERROR)
        return throwClassNotFoundException(execution, "VM error");
    return err;
}

static FlintError nativeIsInstance(FlintExecution &execution) {
    FlintJavaObject *obj = (FlintJavaObject *)execution.stackPopObject();
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaString &typeName = clsObj->getName();
    if(typeName.getCoder() == 0) {
        FlintConstUtf8 *classError;
        FlintError err = execution.flint.isInstanceof(obj, typeName.getText(), typeName.getLength(), &classError);
        if(err == ERR_OK || err == ERR_IS_INSTANCE_FALSE) {
            execution.stackPushInt32((err == ERR_OK) ? 1 : 0);
            return ERR_OK;
        }
        return checkAndThrowForFlintError(execution, err, classError);
    }
    else
        execution.stackPushInt32(0);
    return ERR_OK;
}

static FlintError nativeIsAssignableFrom(FlintExecution &execution) {
    FlintJavaClass *cls = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaClass *thisCls = (FlintJavaClass *)execution.stackPopObject();
    if(cls == NULL || thisCls == NULL)
        return throwNullPointerException(execution);
    uint32_t clsDims, thisDims;
    const FlintConstUtf8 &clsTypeName = cls->getBaseTypeName(execution.flint, &clsDims);
    const FlintConstUtf8 &thisTypeName = thisCls->getBaseTypeName(execution.flint, &thisDims);
    FlintConstUtf8 *classError;
    FlintError err = execution.flint.isInstanceof(clsTypeName, clsDims, thisTypeName, thisDims, &classError);
    if(err == ERR_OK || err == ERR_IS_INSTANCE_FALSE) {
        execution.stackPushInt32((err == ERR_OK) ? 1 : 0);
        return ERR_OK;
    }
    return checkAndThrowForFlintError(execution, err, classError);
}

static FlintError nativeIsInterface(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive())
        execution.stackPushInt32(0);
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintClassLoader *loader;
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
        const FlintClassAccessFlag modifiers = loader->getAccessFlag();
        execution.stackPushInt32((modifiers & CLASS_INTERFACE) ? 1 : 0);
    }
    return ERR_OK;
}

static FlintError nativeIsArray(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    execution.stackPushInt32(clsObj->isArray());
    return ERR_OK;
}

static FlintError nativeIsPrimitive(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    execution.stackPushInt32(clsObj->isPrimitive());
    return ERR_OK;
}

static FlintError nativeGetSuperclass(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive())
        execution.stackPushObject(NULL);
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintClassLoader *loader;
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
        const FlintConstUtf8 *superClass = loader->superClass;
        if(superClass == NULL) {
            execution.stackPushObject(NULL);
            return ERR_OK;
        }
        FlintJavaClass *cls;
        RETURN_IF_ERR(execution.flint.getConstClass(superClass->text, superClass->length, cls));
        execution.stackPushObject(cls);
    }
    return ERR_OK;
}

static FlintError nativeGetInterfaces0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        FlintObjectArray *array;
        RETURN_IF_ERR(execution.flint.getClassArray0(array));
        execution.stackPushObject(array);
    }
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintClassLoader *loader;
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
        uint32_t interfaceCount = loader->getInterfacesCount();
        if(interfaceCount) {
            FlintObjectArray *clsArr;
            err = execution.flint.newObjectArray(*(FlintConstUtf8 *)classClassName, interfaceCount, clsArr);
            if(err != ERR_OK)
                return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)clsArr);
            for(uint32_t i = 0; i < interfaceCount; i++) {
                FlintConstUtf8 &interfaceName = loader->getInterface(i);
                err = execution.flint.getConstClass(interfaceName.text, interfaceName.length, (FlintJavaClass *&)clsArr->getData()[i]);
                if(err != ERR_OK) {
                    execution.flint.freeObject(*clsArr);
                    return err;
                }
            }
            execution.stackPushObject(clsArr);
        }
        else {
            FlintObjectArray *array;
            RETURN_IF_ERR(execution.flint.getClassArray0(array));
            execution.stackPushObject(array);
        }
    }
    return ERR_OK;
}

static FlintError getClass(Flint &flint, const char *typeName, uint16_t length, FlintJavaClass *&cls) {
    if(length == 1) {
        switch(*typeName) {
            case 'Z':   /* boolean */
                flint.getConstClass("boolean", 7, cls);
            case 'C':   /* char */
                return flint.getConstClass("char", 4, cls);
            case 'F':   /* float */
                return flint.getConstClass("float", 5, cls);
            case 'D':   /* double */
                return flint.getConstClass("double", 6, cls);
            case 'B':   /* byte */
                return flint.getConstClass("byte", 4, cls);
            case 'S':   /* short */
                return flint.getConstClass("short", 5, cls);
            case 'I':   /* integer */
                return flint.getConstClass("int", 3, cls);
            default:    /* long */
                return flint.getConstClass("long", 4, cls);
        }
    }
    else
        return flint.getConstClass(typeName, length, cls);
}

static FlintError nativeGetComponentType(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaString &name = clsObj->getName();
    const char *text = name.getText();
    uint8_t coder = name.getCoder();
    uint32_t length = name.getLength();
    if(length > 1 && coder == 0 && text[0] == '[') {
        FlintJavaClass *cls;
        RETURN_IF_ERR(getClass(execution.flint, &text[1], length - 1, cls));
        execution.stackPushObject(cls);
    }
    else
        execution.stackPushInt32(0);
    return ERR_OK;
}

static FlintError nativeGetModifiers(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    int32_t modifiers;
    if(clsObj->isArray() || clsObj->isPrimitive())
        modifiers = 0x0411;
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintClassLoader *loader;
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
        modifiers = loader->getAccessFlag() & 0xFFDF;
    }
    execution.stackPushInt32(modifiers);
    return ERR_OK;
}

static FlintError nativeGetNestHost0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        execution.stackPushObject(clsObj);
        return ERR_OK;
    }
    FlintJavaString &clsName = clsObj->getName();
    char *clsTxt = clsName.getText();
    uint32_t clsLen = clsName.getLength();
    uint32_t clsNestHostLen = 0;
    while((clsNestHostLen < clsLen) && clsTxt[clsNestHostLen] != '$')
        clsNestHostLen++;
    if(clsNestHostLen == clsLen) {
        execution.stackPushObject(clsObj);
        return ERR_OK;
    }
    FlintJavaClass *cls;
    FlintError err = loadClassForName(execution, clsTxt, clsNestHostLen, cls);
    if(err == ERR_OK) {
        execution.stackPushObject(cls);
        return ERR_OK;
    }
    else if(err == ERR_CLASS_NOT_FOUND)
        return throwClassNotFoundException(execution, clsTxt, clsNestHostLen);
    else if(err == ERR_VM_ERROR)
        return throwClassNotFoundException(execution, "VM error");
    return err;
}

static FlintError nativeIsHidden(FlintExecution &execution) {
    // TODO
    return throwUnsupportedOperationException(execution, "isHidden is not implemented in VM");
}

static FlintError getReturnType(Flint &flint, FlintMethodInfo *methodInfo, FlintJavaClass *&cls) {
    FlintConstUtf8 &methodDesc = methodInfo->getDescriptor();
    const char *text = methodDesc.text;
    while(*text++ != ')');
    uint16_t len = methodDesc.length - (text - methodDesc.text);
    return getClass(flint, text, len, cls);
}

static uint8_t getParameterCount(FlintMethodInfo *methodInfo) {
    uint8_t count = 0;
    const char *text = methodInfo->getDescriptor().text;
    while(*text == '(')
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
    return count;
}

static FlintError getParameterTypes(Flint &flint, FlintMethodInfo *methodInfo, FlintObjectArray *classArray0, FlintObjectArray *&parameterTypes) {
    uint8_t count = getParameterCount(methodInfo);
    if(count == 0) {
        parameterTypes = classArray0;
        return ERR_OK;
    }
    FlintObjectArray *array;
    RETURN_IF_ERR(flint.newObjectArray(*(FlintConstUtf8 *)classClassName, count, array));
    FlintJavaObject **data = array->getData();
    count = 0;
    const char *text = methodInfo->getDescriptor().text;
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
        FlintError err = getClass(flint, start, len, (FlintJavaClass *&)data[count]);
        if(err != ERR_OK) {
            flint.freeObject(*array);
            return err;
        }
        count++;
    }
    parameterTypes = array;
    return ERR_OK;
}

static FlintError getExceptionTypes(Flint &flint, FlintMethodInfo *methodInfo, FlintObjectArray *classArray0, FlintObjectArray *&exceptionTypes) {
    if(methodInfo->accessFlag & METHOD_NATIVE) {
        exceptionTypes = classArray0;
        return ERR_OK;
    }
    uint16_t exceptionLength = methodInfo->getExceptionLength();
    if(exceptionLength == 0) {
        exceptionTypes = classArray0;
        return ERR_OK;
    }
    RETURN_IF_ERR(flint.newObjectArray(*(FlintConstUtf8 *)classClassName, exceptionLength, exceptionTypes));
    FlintClassLoader &classLoader = methodInfo->classLoader;
    FlintJavaObject **data = exceptionTypes->getData();
    for(uint16_t i = 0; i < exceptionLength; i++) {
        FlintConstUtf8 &catchType = classLoader.getConstUtf8Class(methodInfo->getException(i)->catchType);
        FlintError err = flint.getConstClass(catchType.text, catchType.length, (FlintJavaClass *&)data[i]);
        if(err != ERR_OK) {
            flint.freeObject(*exceptionTypes);
            return err;
        }
    }
    return ERR_OK;
}

static FlintError nativeGetDeclaredFields0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintClassLoader *loader;
    FlintObjectArray *array;
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        RETURN_IF_ERR(execution.flint.newObjectArray(*(FlintConstUtf8 *)fieldClassName, 0, array));
        execution.stackPushObject(array);
        return ERR_OK;
    }
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
    }
    uint16_t fieldCount = loader->getFieldsCount();
    RETURN_IF_ERR(execution.flint.newObjectArray(*(FlintConstUtf8 *)fieldClassName, fieldCount, array));
    uint32_t clazzIndex = 0, nameIndex = 0, typeIndex = 0, modifiersIndex = 0;
    for(uint16_t i = 0; i < fieldCount; i++) {
        FlintFieldInfo *fieldInfo = loader->getFieldInfo(i);

        FlintJavaObject *field;
        FlintError err = execution.flint.newObject(*(FlintConstUtf8 *)fieldClassName, field);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)field);
        }

        /* clazz */
        FlintFieldsData &fields = field->getFields();
        fields.getFieldObject(*(FlintConstUtf8 *)clazzFieldName, &clazzIndex)->object = clsObj;

        /* name */
        FlintJavaString *nameStr;
        err = execution.flint.getConstString(fieldInfo->getName(), nameStr);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)nameFieldName, &nameIndex)->object = nameStr;

        /* type */
        FlintJavaClass *type;
        err = getClass(execution.flint, fieldInfo->getDescriptor().text, fieldInfo->getDescriptor().length, type);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)typeFieldName, &typeIndex)->object = type;

        /* modifiers */
        fields.getFieldData32(*(FlintConstUtf8 *)modifiersFieldName, &modifiersIndex)->value = (int32_t)fieldInfo->accessFlag & 0x1FFF;

        array->getData()[i] = field;
    }
    execution.stackPushObject(array);
    return ERR_OK;
}

static FlintError nativeGetDeclaredMethods0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintClassLoader *loader;
    FlintObjectArray *array;
    if(clsObj->isArray()) {
        FlintError err = execution.flint.load(*(FlintConstUtf8 *)objectClassName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)objectClassName);
    }
    else if(clsObj->isPrimitive()) {
        RETURN_IF_ERR(execution.flint.newObjectArray(*(FlintConstUtf8 *)methodClassName, 0, array));
        execution.stackPushObject(array);
        return ERR_OK;
    }
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
    }
    uint16_t methodCount = loader->getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo = loader->getMethodInfoWithUnload(i);
        if(methodInfo->getName() != *(FlintConstUtf8 *)constructorName && methodInfo->getName() != *(FlintConstUtf8 *)staticConstructorName)
            count++;
    }
    RETURN_IF_ERR(execution.flint.newObjectArray(*(FlintConstUtf8 *)methodClassName, count, array));
    count = 0;
    uint32_t clazzIndex = 0, nameIndex = 0, returnTypeIndex = 0, parameterTypesIndex = 0, exceptionTypesIndex = 0, modifiersIndex = 0;
    FlintObjectArray *classArray0;
    RETURN_IF_ERR(execution.flint.getClassArray0(classArray0));
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo;
        FlintError err = loader->getMethodInfo(i, methodInfo);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        if(methodInfo->getName() == *(FlintConstUtf8 *)constructorName || methodInfo->getName() == *(FlintConstUtf8 *)staticConstructorName)
            continue;

        FlintJavaObject *method;
        err = execution.flint.newObject(*(FlintConstUtf8 *)methodClassName, method);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)method);
        }

        /* clazz */
        FlintFieldsData &fields = method->getFields();
        fields.getFieldObject(*(FlintConstUtf8 *)clazzFieldName, &clazzIndex)->object = clsObj;

        /* name */
        FlintJavaString *nameStr;
        err = execution.flint.getConstString(methodInfo->getName(), nameStr);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)nameFieldName, &nameIndex)->object = nameStr;

        /* returnType */
        FlintJavaClass *retType;
        err = getReturnType(execution.flint, methodInfo, retType);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)returnTypeFieldName, &returnTypeIndex)->object = retType;

        /* parameterTypes */
        FlintObjectArray *types;
        err = getParameterTypes(execution.flint, methodInfo, classArray0, types);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)parameterTypesFieldName, &parameterTypesIndex)->object = types;
        types->clearProtected();

        /* exceptionTypes */
        err = getExceptionTypes(execution.flint, methodInfo, classArray0, types);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)exceptionTypesFieldName, &exceptionTypesIndex)->object = types;
        types->clearProtected();

        /* modifiers */
        fields.getFieldData32(*(FlintConstUtf8 *)modifiersFieldName, &modifiersIndex)->value = (int32_t)methodInfo->accessFlag & 0x1FFF;

        array->getData()[count++] = method;
    }
    execution.stackPushObject(array);
    return ERR_OK;
}

static FlintError nativeGetDeclaredConstructors0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintClassLoader *loader;
    FlintObjectArray *array;
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        RETURN_IF_ERR(execution.flint.newObjectArray(*(FlintConstUtf8 *)constructorClassName, 0, array));
        execution.stackPushObject(array);
        return ERR_OK;
    }
    else {
        const FlintConstUtf8 &typeName = clsObj->getBaseTypeName(execution.flint);
        FlintError err = execution.flint.load(typeName, loader);
        if(err != ERR_OK)
            return checkAndThrowForFlintError(execution, err, &typeName);
    }
    uint16_t methodCount = loader->getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo = loader->getMethodInfoWithUnload(i);
        if(methodInfo->getName() == *(FlintConstUtf8 *)constructorName)
            count++;
    }
    RETURN_IF_ERR(execution.flint.newObjectArray(*(FlintConstUtf8 *)constructorClassName, count, array));
    count = 0;
    uint32_t clazzIndex = 0, parameterTypesIndex = 0, exceptionTypesIndex = 0, modifiersIndex = 0;
    FlintObjectArray *classArray0;
    RETURN_IF_ERR(execution.flint.getClassArray0(classArray0));
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo = loader->getMethodInfoWithUnload(i);
        if(methodInfo->getName() != *(FlintConstUtf8 *)constructorName)
            continue;

        FlintJavaObject *constructor;
        FlintError err = execution.flint.newObject(*(FlintConstUtf8 *)constructorClassName, constructor);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return checkAndThrowForFlintError(execution, err, (FlintConstUtf8 *)constructor);
        }

        /* clazz */
        FlintFieldsData &fields = constructor->getFields();
        fields.getFieldObject(*(FlintConstUtf8 *)clazzFieldName, &clazzIndex)->object = clsObj;

        /* parameterTypes */
        FlintObjectArray *types;
        err = getParameterTypes(execution.flint, methodInfo, classArray0, types);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)parameterTypesFieldName, &parameterTypesIndex)->object = types;
        types->clearProtected();

        /* exceptionTypes */
        err = getExceptionTypes(execution.flint, methodInfo, classArray0, types);
        if(err != ERR_OK) {
            freeObjectArray(execution.flint, array, i);
            return err;
        }
        fields.getFieldObject(*(FlintConstUtf8 *)exceptionTypesFieldName, &exceptionTypesIndex)->object = types;
        types->clearProtected();

        /* modifiers */
        fields.getFieldData32(*(FlintConstUtf8 *)modifiersFieldName, &modifiersIndex)->value = (int32_t)methodInfo->accessFlag & 0x1FFF;

        array->getData()[count++] = constructor;
    }
    execution.stackPushObject(array);
    return ERR_OK;
}

static FlintError nativeGetDeclaringClass0(FlintExecution &execution) {
    FlintJavaClass *clsObj = (FlintJavaClass *)execution.stackPopObject();
    FlintJavaString &clsName = clsObj->getName();
    const char *text = clsName.getText();
    if(text[0] == '[') {
        execution.stackPushObject(NULL);
        return ERR_OK;
    }
    else {
        int32_t index = clsName.getLength() - 1;
        while(index >= 1 && text[index] != '$')
            index--;
        if(text[index] == '$') {
            FlintJavaClass *cls;
            RETURN_IF_ERR(execution.flint.getConstClass(text, index, cls));
            execution.stackPushObject(cls);
            return ERR_OK;
        }
        execution.stackPushObject(NULL);
    }
    return ERR_OK;
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
    NATIVE_METHOD("\x0C\x00\x9A\x97""getNestHost0",             "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetNestHost0),
    NATIVE_METHOD("\x08\x00\x9C\xA3""isHidden",                 "\x03\x00\x91\x9C""()Z",                                   nativeIsHidden),
    NATIVE_METHOD("\x12\x00\xDA\x76""getDeclaredFields0",       "\x1C\x00\x48\x1C""()[Ljava/lang/reflect/Field;",          nativeGetDeclaredFields0),
    NATIVE_METHOD("\x13\x00\x4B\x12""getDeclaredMethods0",      "\x1D\x00\x12\x57""()[Ljava/lang/reflect/Method;",         nativeGetDeclaredMethods0),
    NATIVE_METHOD("\x18\x00\x0C\xE2""getDeclaredConstructors0", "\x22\x00\x96\xC4""()[Ljava/lang/reflect/Constructor;",    nativeGetDeclaredConstructors0),
    NATIVE_METHOD("\x12\x00\x57\x68""getDeclaringClass0",       "\x13\x00\x0A\x1F""()Ljava/lang/Class;",                   nativeGetDeclaringClass0),
};

const FlintNativeClass CLASS_CLASS = NATIVE_CLASS(classClassName, methods);
