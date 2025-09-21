
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_java_string.h"
#include "flint_native_class.h"

jclass nativeGetPrimitiveClass(FNIEnv *env, jstring name) {
    if(name->getCoder() != 0) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "primitive type name is invalid");
        return NULL;
    }
    return Flint::getPrimitiveClass(env->exec, name->getAscii(), name->getLength());
}

jclass nativeForName(FNIEnv *env, jstring name) {
    if(name == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return NULL;
    }
    uint32_t len = name->getLength();
    if(len >= FILE_NAME_BUFF_SIZE) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "Class name cannot exceed %d characters", FILE_NAME_BUFF_SIZE - 1);
        return NULL;
    }
    char buff[FILE_NAME_BUFF_SIZE];
    const char *txt = name->getAscii();
    uint16_t idx = 0;
    while(idx < len) {
        if(*txt == '/') {
            jclass excpCls = env->findClass("java/lang/ClassNotFoundException");
            env->throwNew(excpCls, "%.*s", name->getLength(), name->getAscii());
            return NULL;
        }
        buff[idx++] = (*txt == '.') ? '/' : *txt;
        txt++;
    }
    buff[idx] = 0;
    return env->findClass(buff);
}

jbool nativeIsInstance(FNIEnv *env, jclass cls, jobject obj) {
    if(obj == NULL) return false;
    return env->isInstanceof(obj, cls);
}

jbool nativeIsAssignableFrom(FNIEnv *env, jclass thisCls, jclass cls) {
    if(cls == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    return env->isAssignableFrom(cls, thisCls);
}

jbool nativeIsInterface(FNIEnv *env, jclass cls) {
    (void)env;
    if(cls->isArray() || cls->isPrimitive()) return false;
    return (cls->getClassLoader()->getAccessFlag() & CLASS_INTERFACE) ? true : false;
}

jbool nativeIsArray(FNIEnv *env, jclass cls) {
    (void)env;
    return cls->isArray();
}

jbool nativeIsPrimitive(FNIEnv *env, jclass cls) {
    (void)env;
    return cls->isPrimitive();
}

jstring nativeInitClassName(FNIEnv *env, jclass cls) {
    char buff[FILE_NAME_BUFF_SIZE];
    uint16_t idx = 0;
    const char *name = cls->getTypeName();
    while(*name) {
        buff[idx++] = (*name != '/') ? *name : '.';
        name++;
    }
    buff[idx] = 0;
    return Flint::getConstString(env->exec, buff);
}

jclass nativeGetSuperclass(FNIEnv *env, jclass cls) {
    if(cls->isArray() || cls->isPrimitive()) return NULL;
    return env->findClass(cls->getClassLoader()->superClass);
}

static jobjectArray getEmptyClassArray(FNIEnv *env) {
    jclass clsOfCls = Flint::getClassOfClass(env->exec);
    FieldObj *field = clsOfCls->getFields()->getFieldObj("EMPTY_CLASS_ARRAY");
    if(field == NULL) {
        jclass excpCls = env->findClass("java/lang/NoSuchFieldError");
        env->throwNew(excpCls, "Could not find the field java/lang/Class.EMPTY_CLASS_ARRAY");
        return NULL;
    }
    return (jobjectArray)field->value;
}

jobjectArray nativeGetInterfaces0(FNIEnv *env, jclass cls) {
    if(cls->isArray() || cls->isPrimitive()) return getEmptyClassArray(env);

    ClassLoader *loader = cls->getClassLoader();
    uint32_t count = loader->getInterfacesCount();
    if(count == 0) return getEmptyClassArray(env);

    jobjectArray clsArr = env->newObjectArray(Flint::getClassOfClass(env->exec), count);
    if(clsArr == NULL) return NULL;

    for(uint32_t i = 0; i < count; i++) {
        jclass ifaceCls = env->findClass(loader->getInterface(i));
        if(ifaceCls == NULL) { env->freeObject(clsArr); return NULL; }
    }
    
    return clsArr;
}

static jclass getPrimitiveClass(FNIEnv *env, char typeDesc) {
    switch(typeDesc) {
        case 'Z': return Flint::getPrimitiveClass(env->exec, "boolean");
        case 'C': return Flint::getPrimitiveClass(env->exec, "char");
        case 'F': return Flint::getPrimitiveClass(env->exec, "float");
        case 'D': return Flint::getPrimitiveClass(env->exec, "double");
        case 'B': return Flint::getPrimitiveClass(env->exec, "byte");
        case 'S': return Flint::getPrimitiveClass(env->exec, "short");
        case 'I': return Flint::getPrimitiveClass(env->exec, "int");
        case 'J': return Flint::getPrimitiveClass(env->exec, "long");
        case 'V': return Flint::getPrimitiveClass(env->exec, "void");
        default:
            env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "Type name is invalid");
            return NULL;
    }
}

jclass nativeGetComponentType(FNIEnv *env, jclass cls) {
    if(cls->isArray()) {
        const char *typeName = cls->getTypeName();
        if(typeName[1] == '[') return env->findClass(&typeName[1]);
        if(typeName[1] == 'L') {
            uint16_t len = 0;
            while(typeName[len + 2] != ';') len++;
            return env->findClass(&typeName[2], len);
        }
        if(typeName[1] == 0) return getPrimitiveClass(env, typeName[0]);
        env->throwNew(env->findClass("java/lang/IllegalArgumentException"), "Type name is invalid");
    }
    return NULL;
}

jint nativeGetModifiers(FNIEnv *env, jclass cls) {
    (void)env;
    if(cls->isArray() || cls->isPrimitive())
        return (CLASS_PUBLIC | CLASS_FINAL | CLASS_ABSTRACT);
    else {
        uint16_t flag = cls->getClassLoader()->getAccessFlag();
        flag &= ~(CLASS_SUPER | CLASS_SYNTHETIC | CLASS_MODULE);
        return flag;
    }
}

jclass nativeGetNestHost0(FNIEnv *env, jclass cls) {
    if(cls->isArray() || cls->isPrimitive()) return cls;
    const char *clsName = cls->getTypeName();
    uint16_t len = 0;
    while(clsName[len] && clsName[len] != '$') len++;
    return env->findClass(clsName, len);
}

jbool nativeIsHidden(FNIEnv *env) {
    // TODO
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "isHidden is not implemented in VM");
    return false;
}

static jclass getReturnType(FNIEnv *env, const char *mtDesc) {
    const char *txt = mtDesc;
    while(*txt++ != ')');
    if(txt[1] == 0) return getPrimitiveClass(env, txt[0]);
    if(txt[0] == 'L') txt++;
    uint16_t len = 0;
    while(txt[len] && txt[len] != ';') len++;
    return env->findClass(txt, len);
}

static uint8_t getParameterCount(const char *mtDesc) {
    uint8_t count = 0;
    const char *txt = mtDesc;
    while(*txt == '(') txt++;
    while(*txt) {
        if(*txt == ')') return count;
        else if(*txt == '[') txt++;
        else {
            count++;
            if(*txt++ == 'L') {
                while(*txt) {
                    if(*txt == ')') return count;
                    else if(*txt == ';') { txt++; break; }
                    txt++;
                }
            }
        }
    }
    return count;
}

static jobjectArray getParameterTypes(FNIEnv *env, const char *mtDesc) {
    uint8_t count = getParameterCount(mtDesc);
    if(count == 0) return getEmptyClassArray(env);
    jobjectArray array = env->newObjectArray(Flint::getClassOfClass(env->exec), count);
    if(array == NULL) return NULL;
    jobject *data = array->getData();
    count = 0;
    const char *txt = mtDesc;
    while(*txt == '(') txt++;
    while(*txt) {
        if(*txt == ')') break;
        const char *start = txt;
        while(*txt == '[') txt++;
        if(*txt == 'L') {
            while(*txt) {
                if(*txt == ')') break;
                else if(*txt == ';') { txt++; break; }
                txt++;
            }
        }
        else txt++;
        uint16_t len = txt - start;
        jclass cls;
        if(len == 1) cls = getPrimitiveClass(env, start[0]);
        else {
            if(start[0] == 'L') { start++; len -= 2; }
            cls = env->findClass(start, len);
        }
        if(cls == NULL) { env->freeObject(array); return NULL; }
        data[count] = cls;
        count++;
    }
    return array;
}

static jobjectArray getExceptionTypes(FNIEnv *env, MethodInfo *mt) {
    if(mt->accessFlag & METHOD_NATIVE)
        return getEmptyClassArray(env);
    uint16_t exceptionLength = mt->getExceptionLength();
    if(exceptionLength == 0)
        return getEmptyClassArray(env);
    jobjectArray excpTypes = env->newObjectArray(Flint::getClassOfClass(env->exec), exceptionLength);
    if(excpTypes == NULL) return NULL;
    ClassLoader *loader = mt->loader;
    jobject *data = excpTypes->getData();
    for(uint16_t i = 0; i < exceptionLength; i++) {
        jclass cls = loader->getConstClass(env->exec, mt->getException(i)->catchType);
        if(cls == NULL) { env->freeObject(excpTypes); return NULL; }
        data[i] = cls;
    }
    return excpTypes;
}

static void supportFreeObjArray(FNIEnv *env, jobjectArray array, uint32_t count) {
    jobject *data = array->getData();
    for(uint32_t i = 0; i < count; i++)
        env->freeObject(data[i]);
    env->freeObject(array);
}

jobjectArray nativeGetDeclaredFields0(FNIEnv *env, jclass cls) {
    jclass fieldCls = env->findClass("java/lang/reflect/Field");
    if(cls->isArray() || cls->isPrimitive()) return env->newObjectArray(fieldCls, 0);

    ClassLoader *loader = cls->getClassLoader();
    uint16_t fieldCount = loader->getFieldsCount();
    jobjectArray array = env->newObjectArray(fieldCls, fieldCount);
    if(array == NULL) return NULL;
    for(uint16_t i = 0; i < fieldCount; i++) {
        FieldInfo *fieldInfo = loader->getFieldInfo(i);

        jobject field = env->newObject(fieldCls);
        if(field == NULL) { supportFreeObjArray(env, array, i); return NULL; }
        array->getData()[i] = field;

        /* clazz */
        field->getFields()->getFieldObj("clazz")->value = cls;

        /* name */
        jstring name = Flint::getConstString(env->exec, fieldInfo->name);
        if(name == NULL) { supportFreeObjArray(env, array, i + 1); return NULL; }
        field->getFields()->getFieldObj("name")->value = name;

        /* type */
        jclass type = env->findClass(fieldInfo->desc);
        if(type == NULL) { supportFreeObjArray(env, array, i + 1); return NULL; }
        field->getFields()->getFieldObj("type")->value = type;

        /* modifiers */
        field->getFields()->getField32("modifiers")->value = (int32_t)fieldInfo->accessFlag & 0x1FFF;
    }
    return array;
}

jobjectArray nativeGetDeclaredMethods0(FNIEnv *env, jclass cls) {
    jclass methodCls = env->findClass("java/lang/reflect/Method");
    if(cls->isPrimitive()) return env->newObjectArray(methodCls, 0);

    ClassLoader *loader = cls->getClassLoader();
    uint16_t methodCount = loader->getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        MethodInfo *methodInfo = loader->getMethodInfo(env->exec, i);
        if(methodInfo == NULL) return NULL;
        if((methodInfo->accessFlag & (METHOD_INIT | METHOD_CLINIT)) == 0) count++;
    }
    jobjectArray array = env->newObjectArray(methodCls, count);
    if(array == NULL) return NULL;
    for(uint16_t midx = 0, aidx = 0; aidx < count; midx++) {
        MethodInfo *methodInfo = loader->getMethodInfo(env->exec, midx);
        if((methodInfo->accessFlag & (METHOD_INIT | METHOD_CLINIT)) != 0) continue;

        jobject method = env->newObject(methodCls);
        if(method == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        array->getData()[aidx++] = method;

        /* clazz */
        method->getFields()->getFieldObj("clazz")->value = cls;

        /* name */
        jstring name = Flint::getConstString(env->exec, methodInfo->name);
        if(name == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        method->getFields()->getFieldObj("name")->value = name;

        /* returnType */
        jclass retType = getReturnType(env, methodInfo->desc);
        if(retType == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        method->getFields()->getFieldObj("returnType")->value = retType;

        /* parameterTypes */
        jobjectArray types = getParameterTypes(env, methodInfo->desc);
        if(types == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        method->getFields()->getFieldObj("parameterTypes")->value = types;
        types->clearProtected();

        /* exceptionTypes */
        types = getExceptionTypes(env, methodInfo);
        if(types == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        method->getFields()->getFieldObj("exceptionTypes")->value = types;
        types->clearProtected();

        /* modifiers */
        method->getFields()->getField32("modifiers")->value = (int32_t)methodInfo->accessFlag & 0x1FFF;
    }
    return array;
}

jobjectArray nativeGetDeclaredConstructors0(FNIEnv *env, jclass cls) {
    jclass ctorCls = env->findClass("java/lang/reflect/Constructor");
    if(cls->isArray() || cls->isPrimitive()) return env->newObjectArray(ctorCls, 0);

    ClassLoader *loader = cls->getClassLoader();
    uint16_t methodCount = loader->getMethodsCount();
    uint16_t count = 0;
    for(uint16_t i = 0; i < methodCount; i++) {
        MethodInfo *methodInfo = loader->getMethodInfo(env->exec, i);
        if(methodInfo == NULL) return NULL;
        if(methodInfo->accessFlag & METHOD_INIT) count++;
    }
    jobjectArray array = env->newObjectArray(ctorCls, count);
    if(array == NULL) return NULL;
    for(uint16_t midx = 0, aidx = 0; aidx < count; midx++) {
        MethodInfo *methodInfo = loader->getMethodInfo(env->exec, midx);
        if(!(methodInfo->accessFlag & METHOD_INIT)) continue;

        jobject ctor = env->newObject(ctorCls);
        if(ctor == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        array->getData()[aidx++] = ctor;

        /* clazz */
        ctor->getFields()->getFieldObj("clazz")->value = cls;

        /* parameterTypes */
        jobjectArray types = getParameterTypes(env, methodInfo->desc);
        if(types == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        ctor->getFields()->getFieldObj("parameterTypes")->value = types;
        types->clearProtected();

        /* exceptionTypes */
        types = getExceptionTypes(env, methodInfo);
        if(types == NULL) { supportFreeObjArray(env, array, aidx); return NULL; }
        ctor->getFields()->getFieldObj("exceptionTypes")->value = types;
        types->clearProtected();

        /* modifiers */
        ctor->getFields()->getField32("modifiers")->value = (int32_t)methodInfo->accessFlag & 0x1FFF;
    }
    return array;
}

jclass nativeGetDeclaringClass0(FNIEnv *env, jclass cls) {
    if(cls->isArray() || cls->isPrimitive()) return NULL;
    const char *clsName = cls->getTypeName();
    uint16_t len = strlen(clsName);
    while(len > 0 && clsName[len - 1] != '$') len--;
    if(len > 0 && clsName[len - 1] == '$') return env->findClass(clsName, len - 1);
    return NULL;
}
