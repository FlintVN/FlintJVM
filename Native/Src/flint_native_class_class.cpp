
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_const_name_base.h"
#include "flint_native_class_class.h"
#include "flint_throw_support.h"

static void freeObjectArray(Flint &flint, JObjectArray *array, uint32_t length) {
    for(uint32_t i = 0; i < length; i++)
        flint.freeObject(array->getData()[i]);
    flint.freeObject(array);
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
    if(FlintAPI::IO::finfo(buff, NULL_PTR, NULL_PTR) != FILE_RESULT_OK)
        return ERR_CLASS_NOT_FOUND;
    return ERR_OK;
}

static FlintError nativeGetPrimitiveClass(FlintExecution *exec) {
    JString *str = (JString *)exec->stackPopObject();
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
            auto cls = exec->flint.getConstClass(str->getText(), len);
            RETURN_IF_ERR(cls.err);
            exec->stackPushObject(cls.value);
            return ERR_OK;
        }
    }
    return throwIllegalArgumentException(exec, "primitive type name is invalid");
}

static FlintError nativeForName(FlintExecution *exec) {
    JString *type = (JString *)exec->stackPopObject();
    if(type == NULL_PTR)
        return throwNullPointerException(exec);
    RETURN_IF_ERR(checkClassIsExist(type->getText(), type->getLength()));
    auto cls = exec->flint.getConstClass(type->getText(), type->getLength());
    if(cls.err == ERR_OK) {
        exec->stackPushObject(cls.value);
        return ERR_OK;
    }
    else if(cls.err == ERR_CLASS_NOT_FOUND)
        return throwClassNotFoundException(exec, type);
    else if(cls.err == ERR_VM_ERROR)
        return throwClassNotFoundException(exec, "VM error");
    return cls.err;
}

static FlintError nativeIsInstance(FlintExecution *exec) {
    JObject *obj = (JObject *)exec->stackPopObject();
    JClass *clsObj = (JClass *)exec->stackPopObject();
    JString *typeName = clsObj->getName();
    if(typeName->getCoder() == 0) {
        auto res = exec->flint.isInstanceof(obj, typeName->getText(), typeName->getLength());
        if(res.err == ERR_OK) {
            exec->stackPushInt32(res.value ? 1 : 0);
            return ERR_OK;
        }
        return checkAndThrowForFlintError(exec, res.err, res.getErrorMsg(), res.getErrorMsgLength());
    }
    else
        exec->stackPushInt32(0);
    return ERR_OK;
}

static FlintError nativeIsAssignableFrom(FlintExecution *exec) {
    JClass *cls = (JClass *)exec->stackPopObject();
    JClass *thisCls = (JClass *)exec->stackPopObject();
    if(cls == NULL_PTR || thisCls == NULL_PTR)
        return throwNullPointerException(exec);
    uint32_t clsDims, thisDims;
    auto clsTypeName = cls->getBaseTypeName(exec->flint, &clsDims);
    if(clsTypeName.err != ERR_OK)
        return checkAndThrowForFlintError(exec, clsTypeName.err, clsTypeName.getErrorMsg(), clsTypeName.getErrorMsgLength());
    auto thisTypeName = thisCls->getBaseTypeName(exec->flint, &thisDims);
    if(thisTypeName.err != ERR_OK)
        return checkAndThrowForFlintError(exec, thisTypeName.err, thisTypeName.getErrorMsg(), thisTypeName.getErrorMsgLength());
    auto res = exec->flint.isInstanceof(*clsTypeName.value, clsDims, *thisTypeName.value, thisDims);
    if(res.err) {
        exec->stackPushInt32(res.value ? 1 : 0);
        return ERR_OK;
    }
    return checkAndThrowForFlintError(exec, res.err, res.getErrorMsg(), res.getErrorMsgLength());
}

static FlintError nativeIsInterface(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive())
        exec->stackPushInt32(0);
    else {
        auto typeName = clsObj->getBaseTypeName(exec->flint);
        if(typeName.err != ERR_OK)
            return checkAndThrowForFlintError(exec, typeName.err, typeName.value);
        auto loader = exec->flint.load(typeName.value->text);
        if(loader.err != ERR_OK)
            return checkAndThrowForFlintError(exec, loader.err, typeName.value);
        const FlintClassAccessFlag modifiers = loader.value->getAccessFlag();
        exec->stackPushInt32((modifiers & CLASS_INTERFACE) ? 1 : 0);
    }
    return ERR_OK;
}

static FlintError nativeIsArray(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    exec->stackPushInt32(clsObj->isArray());
    return ERR_OK;
}

static FlintError nativeIsPrimitive(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    exec->stackPushInt32(clsObj->isPrimitive());
    return ERR_OK;
}

static FlintError nativeGetSuperclass(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive())
        exec->stackPushObject(NULL_PTR);
    else {
        auto typeName = clsObj->getBaseTypeName(exec->flint);
        if(typeName.err != ERR_OK)
            return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
        auto loader = exec->flint.load(typeName.value->text);
        if(loader.err != ERR_OK)
            return checkAndThrowForFlintError(exec, loader.err, typeName.value);
        FlintConstUtf8 *superClass = loader.value->superClass;
        if(superClass == NULL_PTR) {
            exec->stackPushObject(NULL_PTR);
            return ERR_OK;
        }
        auto cls = exec->flint.getConstClass(superClass->text, superClass->length);
        RETURN_IF_ERR(cls.err);
        exec->stackPushObject(cls.value);
    }
    return ERR_OK;
}

static FlintError nativeGetInterfaces0(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        auto array = exec->flint.getClassArray0();
        RETURN_IF_ERR(array.err);
        exec->stackPushObject(array.value);
    }
    else {
        auto typeName = clsObj->getBaseTypeName(exec->flint);
        if(typeName.err)
            return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
        auto loader = exec->flint.load(typeName.value->text);
        if(loader.err != ERR_OK)
            return checkAndThrowForFlintError(exec, loader.err, typeName.value);
        uint32_t interfaceCount = loader.value->getInterfacesCount();
        if(interfaceCount) {
            auto clsArr = exec->flint.newObjectArray((FlintConstUtf8 *)classClassName, interfaceCount);
            if(clsArr.err != ERR_OK)
                return checkAndThrowForFlintError(exec, clsArr.err, clsArr.getErrorMsg(), clsArr.getErrorMsgLength());
            for(uint32_t i = 0; i < interfaceCount; i++) {
                FlintConstUtf8 &interfaceName = loader.value->getInterface(i);
                auto cls = exec->flint.getConstClass(interfaceName.text, interfaceName.length);
                if(cls.err != ERR_OK) {
                    exec->flint.freeObject(clsArr.value);
                    return checkAndThrowForFlintError(exec, cls.err, cls.getErrorMsg(), cls.getErrorMsgLength());
                }
                clsArr.value->getData()[i] = cls.value;
            }
            exec->stackPushObject(clsArr.value);
        }
        else {
            auto array = exec->flint.getClassArray0();
            RETURN_IF_ERR(array.err);
            exec->stackPushObject(array.value);
        }
    }
    return ERR_OK;
}

static FlintResult<JClass> getClass(Flint &flint, const char *typeName, uint16_t length) {
    if(length == 1) {
        switch(*typeName) {
            case 'Z':   /* boolean */
                return flint.getConstClass("boolean", 7);
            case 'C':   /* char */
                return flint.getConstClass("char", 4);
            case 'F':   /* float */
                return flint.getConstClass("float", 5);
            case 'D':   /* double */
                return flint.getConstClass("double", 6);
            case 'B':   /* byte */
                return flint.getConstClass("byte", 4);
            case 'S':   /* short */
                return flint.getConstClass("short", 5);
            case 'I':   /* integer */
                return flint.getConstClass("int", 3);
            default:    /* long */
                return flint.getConstClass("long", 4);
        }
    }
    else
        return flint.getConstClass(typeName, length);
}

static FlintError nativeGetComponentType(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    JString *name = clsObj->getName();
    const char *text = name->getText();
    uint8_t coder = name->getCoder();
    uint32_t length = name->getLength();
    if(length > 1 && coder == 0 && text[0] == '[') {
        auto cls = getClass(exec->flint, &text[1], length - 1);
        RETURN_IF_ERR(cls.err);
        exec->stackPushObject(cls.value);
    }
    else
        exec->stackPushInt32(0);
    return ERR_OK;
}

static FlintError nativeGetModifiers(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    int32_t modifiers;
    if(clsObj->isArray() || clsObj->isPrimitive())
        modifiers = 0x0411;
    else {
        auto typeName = clsObj->getBaseTypeName(exec->flint);
        if(typeName.err != ERR_OK)
            return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
        auto loader = exec->flint.load(typeName.value->text);
        if(loader.err != ERR_OK)
            return checkAndThrowForFlintError(exec, loader.err, typeName.value);
        modifiers = loader.value->getAccessFlag() & 0xFFDF;
    }
    exec->stackPushInt32(modifiers);
    return ERR_OK;
}

static FlintError nativeGetNestHost0(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        exec->stackPushObject(clsObj);
        return ERR_OK;
    }
    JString *clsName = clsObj->getName();
    char *clsTxt = clsName->getText();
    uint32_t clsLen = clsName->getLength();
    uint32_t clsNestHostLen = 0;
    while((clsNestHostLen < clsLen) && clsTxt[clsNestHostLen] != '$')
        clsNestHostLen++;
    if(clsNestHostLen == clsLen) {
        exec->stackPushObject(clsObj);
        return ERR_OK;
    }
    RETURN_IF_ERR(checkClassIsExist(clsTxt, clsNestHostLen));
    auto cls = exec->flint.getConstClass(clsTxt, clsNestHostLen);
    if(cls.err == ERR_OK) {
        exec->stackPushObject(cls.value);
        return ERR_OK;
    }
    else if(cls.err == ERR_CLASS_NOT_FOUND)
        return throwClassNotFoundException(exec, clsTxt, clsNestHostLen);
    else if(cls.err == ERR_VM_ERROR)
        return throwClassNotFoundException(exec, "VM error");
    return cls.err;
}

static FlintError nativeIsHidden(FlintExecution *exec) {
    // TODO
    return throwUnsupportedOperationException(exec, "isHidden is not implemented in VM");
}

static FlintResult<JClass> getReturnType(Flint &flint, FlintMethodInfo *methodInfo) {
    FlintConstUtf8 &methodDesc = methodInfo->getDescriptor();
    const char *text = methodDesc.text;
    while(*text++ != ')');
    uint16_t len = methodDesc.length - (text - methodDesc.text);
    return getClass(flint, text, len);
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

static FlintResult<JObjectArray> getParameterTypes(Flint &flint, FlintMethodInfo *methodInfo, FlintResult<JObjectArray> &classArray0) {
    uint8_t count = getParameterCount(methodInfo);
    if(count == 0)
        return classArray0;
    auto array = flint.newObjectArray((FlintConstUtf8 *)classClassName, count);
    if(array.err != ERR_OK)
        return array;
    JObject **data = array.value->getData();
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
        auto cls = getClass(flint, start, len);
        if(cls.err != ERR_OK) {
            flint.freeObject(array.value);
            return *(FlintResult<JObjectArray> *)&cls;
        }
        data[count] = cls.value;
        count++;
    }
    return array;
}

static FlintResult<JObjectArray> getExceptionTypes(Flint &flint, FlintMethodInfo *methodInfo, FlintResult<JObjectArray> &classArray0) {
    if(methodInfo->accessFlag & METHOD_NATIVE)
        return classArray0;
    uint16_t exceptionLength = methodInfo->getExceptionLength();
    if(exceptionLength == 0)
        return classArray0;
    auto excpTypes = flint.newObjectArray((FlintConstUtf8 *)classClassName, exceptionLength);
    if(excpTypes.err != ERR_OK)
        return excpTypes;
    FlintClassLoader &classLoader = methodInfo->classLoader;
    JObject **data = excpTypes.value->getData();
    for(uint16_t i = 0; i < exceptionLength; i++) {
        FlintConstUtf8 &catchType = classLoader.getConstUtf8Class(methodInfo->getException(i)->catchType);
        auto cls = flint.getConstClass(catchType.text, catchType.length);
        if(cls.err != ERR_OK) {
            flint.freeObject(excpTypes.value);
            return *(FlintResult<JObjectArray> *)&cls;
        }
        data[i] = cls.value;
    }
    return excpTypes;
}

static FlintError nativeGetDeclaredFields0(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        auto array = exec->flint.newObjectArray((FlintConstUtf8 *)fieldClassName, 0);
        RETURN_IF_ERR(array.err);
        exec->stackPushObject(array.value);
        return ERR_OK;
    }
    auto typeName = clsObj->getBaseTypeName(exec->flint);
    if(typeName.err != ERR_OK)
        return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
    auto loader = exec->flint.load(typeName.value->text);
    if(loader.err != ERR_OK)
        return checkAndThrowForFlintError(exec, loader.err, typeName.value);
    uint16_t fieldCount = loader.value->getFieldsCount();
    auto array = exec->flint.newObjectArray((FlintConstUtf8 *)fieldClassName, fieldCount);
    RETURN_IF_ERR(array.err);
    uint32_t clazzIndex = 0, nameIndex = 0, typeIndex = 0, modifiersIndex = 0;
    for(uint16_t i = 0; i < fieldCount; i++) {
        FlintFieldInfo *fieldInfo = loader.value->getFieldInfo(i);

        auto field = exec->flint.newObject((FlintConstUtf8 *)fieldClassName);
        if(field.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, field.err, field.getErrorMsg(), field.getErrorMsgLength());
        }

        /* clazz */
        FlintFieldsData &fields = field.value->getFields();
        fields.getFieldObject(*(FlintConstUtf8 *)clazzFieldName, &clazzIndex)->object = clsObj;

        /* name */
        auto nameStr = exec->flint.getConstString(fieldInfo->getName());
        if(nameStr.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, nameStr.err, nameStr.getErrorMsg(), nameStr.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)nameFieldName, &nameIndex)->object = nameStr.value;

        /* type */
        auto type = getClass(exec->flint, fieldInfo->getDescriptor().text, fieldInfo->getDescriptor().length);
        if(type.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, type.err, type.getErrorMsg(), type.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)typeFieldName, &typeIndex)->object = type.value;

        /* modifiers */
        fields.getFieldData32(*(FlintConstUtf8 *)modifiersFieldName, &modifiersIndex)->value = (int32_t)fieldInfo->accessFlag & 0x1FFF;

        array.value->getData()[i] = field.value;
    }
    exec->stackPushObject(array.value);
    return ERR_OK;
}

static FlintError nativeGetDeclaredMethods0(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isPrimitive()) {
        auto array = exec->flint.newObjectArray((FlintConstUtf8 *)methodClassName, 0);
        RETURN_IF_ERR(array.err);
        exec->stackPushObject(array.value);
        return ERR_OK;
    }
    FlintConstUtf8 *typeName;
    if(clsObj->isArray())
        typeName = (FlintConstUtf8 *)objectClassName;
    else {
        auto tmp = clsObj->getBaseTypeName(exec->flint);
        if(tmp.err != ERR_OK)
            return checkAndThrowForFlintError(exec, tmp.err, tmp.getErrorMsg(), tmp.getErrorMsgLength());
        typeName = tmp.value;
    }

    auto loader = exec->flint.load(typeName->text);
    if(loader.err != ERR_OK)
        return checkAndThrowForFlintError(exec, loader.err, typeName);
    uint16_t methodCount = loader.value->getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo = loader.value->getMethodInfoWithUnload(i);
        if(methodInfo->getName() != *(FlintConstUtf8 *)constructorName && methodInfo->getName() != *(FlintConstUtf8 *)staticConstructorName)
            count++;
    }
    auto array = exec->flint.newObjectArray((FlintConstUtf8 *)methodClassName, count);
    RETURN_IF_ERR(array.err);
    count = 0;
    uint32_t clazzIndex = 0, nameIndex = 0, returnTypeIndex = 0, parameterTypesIndex = 0, exceptionTypesIndex = 0, modifiersIndex = 0;
    auto classArray0 = exec->flint.getClassArray0();
    RETURN_IF_ERR(classArray0.err);
    for(uint16_t i = 0; i < methodCount; i++) {
        auto methodInfo = loader.value->getMethodInfo(i);
        if(methodInfo.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, methodInfo.err, methodInfo.getErrorMsg(), methodInfo.getErrorMsgLength());
        }
        if(methodInfo.value->getName() == *(FlintConstUtf8 *)constructorName || methodInfo.value->getName() == *(FlintConstUtf8 *)staticConstructorName)
            continue;

        auto method = exec->flint.newObject((FlintConstUtf8 *)methodClassName);
        if(method.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, method.err, method.getErrorMsg(), method.getErrorMsgLength());
        }

        /* clazz */
        FlintFieldsData &fields = method.value->getFields();
        fields.getFieldObject(*(FlintConstUtf8 *)clazzFieldName, &clazzIndex)->object = clsObj;

        /* name */
        auto nameStr = exec->flint.getConstString(methodInfo.value->getName());
        if(nameStr.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, nameStr.err, nameStr.getErrorMsg(), nameStr.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)nameFieldName, &nameIndex)->object = nameStr.value;

        /* returnType */
        auto retType = getReturnType(exec->flint, methodInfo.value);
        if(retType.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, retType.err, retType.getErrorMsg(), retType.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)returnTypeFieldName, &returnTypeIndex)->object = retType.value;

        /* parameterTypes */
        auto types = getParameterTypes(exec->flint, methodInfo.value, classArray0);
        if(types.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, types.err, types.getErrorMsg(), types.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)parameterTypesFieldName, &parameterTypesIndex)->object = types.value;
        types.value->clearProtected();

        /* exceptionTypes */
        types = getExceptionTypes(exec->flint, methodInfo.value, classArray0);
        if(types.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, types.err, types.getErrorMsg(), types.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)exceptionTypesFieldName, &exceptionTypesIndex)->object = types.value;
        types.value->clearProtected();

        /* modifiers */
        fields.getFieldData32(*(FlintConstUtf8 *)modifiersFieldName, &modifiersIndex)->value = (int32_t)methodInfo.value->accessFlag & 0x1FFF;

        array.value->getData()[count++] = method.value;
    }
    exec->stackPushObject(array.value);
    return ERR_OK;
}

static FlintError nativeGetDeclaredConstructors0(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    if(clsObj->isArray() || clsObj->isPrimitive()) {
        auto array = exec->flint.newObjectArray((FlintConstUtf8 *)constructorClassName, 0);
        RETURN_IF_ERR(array.err);
        exec->stackPushObject(array.value);
        return ERR_OK;
    }
    auto typeName = clsObj->getBaseTypeName(exec->flint);
    if(typeName.err != ERR_OK)
        return checkAndThrowForFlintError(exec, typeName.err, typeName.getErrorMsg(), typeName.getErrorMsgLength());
    auto loader = exec->flint.load(typeName.value->text);
    if(loader.err != ERR_OK)
        return checkAndThrowForFlintError(exec, loader.err, typeName.value);
    uint16_t methodCount = loader.value->getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo = loader.value->getMethodInfoWithUnload(i);
        if(methodInfo->getName() == *(FlintConstUtf8 *)constructorName)
            count++;
    }
    auto array = exec->flint.newObjectArray((FlintConstUtf8 *)constructorClassName, count);
    RETURN_IF_ERR(array.err);
    count = 0;
    uint32_t clazzIndex = 0, parameterTypesIndex = 0, exceptionTypesIndex = 0, modifiersIndex = 0;
    auto classArray0 = exec->flint.getClassArray0();
    RETURN_IF_ERR(classArray0.err);
    for(uint16_t i = 0; i < methodCount; i++) {
        FlintMethodInfo *methodInfo = loader.value->getMethodInfoWithUnload(i);
        if(methodInfo->getName() != *(FlintConstUtf8 *)constructorName)
            continue;

        auto constructor = exec->flint.newObject((FlintConstUtf8 *)constructorClassName);
        if(constructor.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, constructor.err, constructor.getErrorMsg(), constructor.getErrorMsgLength());
        }

        /* clazz */
        FlintFieldsData &fields = constructor.value->getFields();
        fields.getFieldObject(*(FlintConstUtf8 *)clazzFieldName, &clazzIndex)->object = clsObj;

        /* parameterTypes */
        auto types = getParameterTypes(exec->flint, methodInfo, classArray0);
        if(types.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, types.err, types.getErrorMsg(), types.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)parameterTypesFieldName, &parameterTypesIndex)->object = types.value;
        types.value->clearProtected();

        /* exceptionTypes */
        types = getExceptionTypes(exec->flint, methodInfo, classArray0);
        if(types.err != ERR_OK) {
            freeObjectArray(exec->flint, array.value, i);
            return checkAndThrowForFlintError(exec, types.err, types.getErrorMsg(), types.getErrorMsgLength());
        }
        fields.getFieldObject(*(FlintConstUtf8 *)exceptionTypesFieldName, &exceptionTypesIndex)->object = types.value;
        types.value->clearProtected();

        /* modifiers */
        fields.getFieldData32(*(FlintConstUtf8 *)modifiersFieldName, &modifiersIndex)->value = (int32_t)methodInfo->accessFlag & 0x1FFF;

        array.value->getData()[count++] = constructor.value;
    }
    exec->stackPushObject(array.value);
    return ERR_OK;
}

static FlintError nativeGetDeclaringClass0(FlintExecution *exec) {
    JClass *clsObj = (JClass *)exec->stackPopObject();
    JString *clsName = clsObj->getName();
    const char *text = clsName->getText();
    if(text[0] == '[') {
        exec->stackPushObject(NULL_PTR);
        return ERR_OK;
    }
    else {
        int32_t index = clsName->getLength() - 1;
        while(index >= 1 && text[index] != '$')
            index--;
        if(text[index] == '$') {
            auto cls = exec->flint.getConstClass(text, index);
            RETURN_IF_ERR(cls.err);
            exec->stackPushObject(cls.value);
            return ERR_OK;
        }
        exec->stackPushObject(NULL_PTR);
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
