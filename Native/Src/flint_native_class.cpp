
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_java_string.h"
#include "flint_native_class.h"

jclass NativeClass_GetPrimitiveClass(FNIEnv *env, jstring name) {
    if(name->getCoder() != 0) {
        jclass excpCls = env->findClass("java/lang/IllegalArgumentException");
        env->throwNew(excpCls, "primitive type name is invalid");
        return NULL;
    }
    return Flint::getPrimitiveClass(env->exec, name->getAscii(), name->getLength());
}

jclass NativeClass_ForName(FNIEnv *env, jstring name) {
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

jbool NativeClass_IsInstance(FNIEnv *env, jclass cls, jobject obj) {
    if(obj == NULL) return false;
    return env->isInstanceof(obj, cls);
}

jbool NativeClass_IsAssignableFrom(FNIEnv *env, jclass thisCls, jclass cls) {
    if(cls == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return false;
    }
    return env->isAssignableFrom(cls, thisCls);
}

jbool NativeClass_IsInterface(FNIEnv *env, jclass cls) {
    (void)env;
    if(cls->isArray() || cls->isPrimitive()) return false;
    return (cls->getClassLoader()->getAccessFlag() & CLASS_INTERFACE) ? true : false;
}

jbool NativeClass_IsArray(FNIEnv *env, jclass cls) {
    (void)env;
    return cls->isArray();
}

jbool NativeClass_IsPrimitive(FNIEnv *env, jclass cls) {
    (void)env;
    return cls->isPrimitive();
}

jstring NativeClass_InitClassName(FNIEnv *env, jclass cls) {
    char buff[FILE_NAME_BUFF_SIZE];
    uint16_t idx = 0;
    const char *name = cls->getTypeName();
    while(*name) {
        buff[idx++] = (*name != '/') ? *name : '.';
        name++;
    }
    buff[idx] = 0;
    jstring str = Flint::getConstString(env->exec, buff);
    cls->getField(env->exec, "name")->setObj(str);
    return str;
}

jclass NativeClass_GetSuperclass(FNIEnv *env, jclass cls) {
    if(cls->isArray() || cls->isPrimitive()) return NULL;
    return cls->getClassLoader()->getSuperClass(env->exec);
}

static jobjectArray getEmptyClassArray(FNIEnv *env) {
    jclass clsOfCls = Flint::getClassOfClass(env->exec);
    FieldValue *field = clsOfCls->getClassLoader()->getStaticField(env->exec, "EMPTY_CLASS_ARRAY");
    if(field == NULL) return NULL;
    return (jobjectArray)field->getObj();
}

jobjectArray NativeClass_GetInterfaces0(FNIEnv *env, jclass cls) {
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

static jclass findClassOrPrimitive(FNIEnv *env, const char *desc, uint16_t length) {
    if(length == 1) switch(desc[0]) {
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
    if(desc[0] == 'L') {
        desc++;
        length--;
        while(desc[length - 1] == ';') length--;
    }
    return env->findClass(desc, length);
}

jclass NativeClass_GetComponentType(FNIEnv *env, jclass cls) {
    if(cls->isArray()) {
        const char *typeName = cls->getTypeName();
        uint16_t len = GetArgNameLength(typeName);
        return findClassOrPrimitive(env, typeName, len);
    }
    return NULL;
}

jint NativeClass_GetModifiers(FNIEnv *env, jclass cls) {
    (void)env;
    if(cls->isArray() || cls->isPrimitive())
        return (CLASS_PUBLIC | CLASS_FINAL | CLASS_ABSTRACT);
    else {
        uint16_t flag = cls->getClassLoader()->getAccessFlag();
        flag &= ~(CLASS_SUPER | CLASS_SYNTHETIC | CLASS_MODULE);
        return flag;
    }
}

jclass NativeClass_GetNestHost0(FNIEnv *env, jclass cls) {
    return cls->getNestHost(env->exec);
}

jobjectArray NativeClass_GetNestMembers0(FNIEnv *env, jclass cls) {
    jobjectArray array;
    if(cls->isArray() || cls->isPrimitive()) {
        array = env->newObjectArray(env->findClass("java/lang/Class"), 1);
        if(array == NULL) return NULL;
        array->getData()[0] = cls;
        return array;
    }
    jclass nestHost = cls->getNestHost(env->exec);
    if(nestHost == NULL) return NULL;
    uint16_t membersCount = nestHost->getNestMembersCount();
    array = env->newObjectArray(env->findClass("java/lang/Class"), membersCount + 1);
    if(array == NULL) return NULL;
    array->getData()[0] = nestHost;
    for(uint16_t i = 0; i < membersCount; i++) {
        jclass clsMember = nestHost->getNestMember(env->exec, i);
        if(clsMember == NULL) return NULL;
        array->getData()[i + 1] = clsMember;
    }
    return array;
}

jbool NativeClass_IsHidden(FNIEnv *env) {
    // TODO
    return false;
}

static jclass getReturnType(FNIEnv *env, const char *mtDesc) {
    const char *txt = mtDesc;
    while(*txt++ != ')');
    return findClassOrPrimitive(env, txt, GetArgNameLength(txt));
}

static jobjectArray getParameterTypes(FNIEnv *env, const char *mtDesc) {
    uint8_t count = GetArgCount(mtDesc);
    if(count == 0) return getEmptyClassArray(env);
    jobjectArray array = env->newObjectArray(Flint::getClassOfClass(env->exec), count);
    if(array == NULL) return NULL;
    mtDesc = GetNextArgName(mtDesc);
    for(uint8_t i = 0; i < count; i++) {
        uint16_t len = GetArgNameLength(mtDesc);
        array->getData()[i] = findClassOrPrimitive(env, mtDesc, len);
        mtDesc += len;
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

jobjectArray NativeClass_GetDeclaredFields0(FNIEnv *env, jclass cls) {
    jclass fieldCls = env->findClass("java/lang/reflect/Field");
    if(cls->isArray() || cls->isPrimitive()) return env->newObjectArray(fieldCls, 0);
    jmethodId ctorId = env->getConstructorId(fieldCls, "(Ljava/lang/Class;Ljava/lang/String;Ljava/lang/Class;I)V");
    if(ctorId == NULL) return NULL;

    ClassLoader *loader = cls->getClassLoader();
    uint16_t fieldCount = loader->getFieldsCount();
    jobjectArray array = env->newObjectArray(fieldCls, fieldCount);
    if(array == NULL) return NULL;
    for(uint16_t i = 0; i < fieldCount; i++) {
        bool isOk = false;
        do {
            FieldInfo *fieldInfo = loader->getFieldInfo(i);

            /* name */
            jstring name = Flint::getConstString(env->exec, fieldInfo->name);
            if(name == NULL) break;

            /* type */
            jclass type = fieldInfo->desc[1] == 0 ? findClassOrPrimitive(env, fieldInfo->desc, 1) : env->findClass(fieldInfo->desc);
            if(type == NULL) break;

            jobject field = env->newObject(fieldCls, ctorId, cls, name, type, (int32_t)fieldInfo->accessFlag & 0x1FFF);
            if(field == NULL) break;
            field->getField(env->exec, "entry")->setInt32(i);

            array->getData()[i] = field;
            isOk = true;
        } while(0);

        if(!isOk) { supportFreeObjArray(env, array, i + 1); return NULL; }
    }
    return array;
}

jobjectArray NativeClass_GetDeclaredMethods0(FNIEnv *env, jclass cls) {
    jclass methodCls = env->findClass("java/lang/reflect/Method");
    if(cls->isPrimitive()) return env->newObjectArray(methodCls, 0);
    jmethodId ctorId = env->getConstructorId(methodCls, "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Class;Ljava/lang/Class;[Ljava/lang/Class;I)V");
    if(ctorId == NULL) return NULL;

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

        bool isOk = false;
        do {
            /* name */
            jstring name = Flint::getConstString(env->exec, methodInfo->name);
            if(name == NULL) break;

            /* returnType */
            jclass retType = getReturnType(env, methodInfo->desc);
            if(retType == NULL) break;

            /* parameterTypes */
            jobjectArray ptypes = getParameterTypes(env, methodInfo->desc);
            if(ptypes == NULL) break;

            /* exceptionTypes */
            jobjectArray etypes = getExceptionTypes(env, methodInfo);
            if(etypes == NULL) break;
            /* modifiers */

            jobject method = env->newObject(methodCls, ctorId, cls, name, ptypes, retType, etypes, (int32_t)methodInfo->accessFlag & 0x1FFF);
            if(method == NULL) {
                ptypes->clearProtected();
                etypes->clearProtected();
                break;
            }
            method->getField(env->exec, "entry")->setInt32((int32_t)methodInfo);

            array->getData()[aidx++] = method;
            isOk = true;
        } while(0);

        if(!isOk) { supportFreeObjArray(env, array, aidx); return NULL; }
    }
    return array;
}

jobjectArray NativeClass_GetDeclaredConstructors0(FNIEnv *env, jclass cls) {
    jclass ctorCls = env->findClass("java/lang/reflect/Constructor");
    if(cls->isArray() || cls->isPrimitive()) return env->newObjectArray(ctorCls, 0);
    jmethodId ctorId = env->getConstructorId(ctorCls, "(Ljava/lang/Class;[Ljava/lang/Class;[Ljava/lang/Class;I)V");
    if(ctorId == NULL) return NULL;

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

        bool isOk = false;
        do {
            /* parameterTypes */
            jobjectArray ptypes = getParameterTypes(env, methodInfo->desc);
            if(ptypes == NULL) break;

            /* exceptionTypes */
            jobjectArray etypes = getExceptionTypes(env, methodInfo);
            if(etypes == NULL) break;

            jobject ctor = env->newObject(ctorCls, ctorId, cls, ptypes, etypes, (int32_t)methodInfo->accessFlag & 0x1FFF);
            if(ctor == NULL) {
                ptypes->clearProtected();
                etypes->clearProtected();
                break;
            }
            ctor->getField(env->exec, "entry")->setInt32((int32_t)methodInfo);

            array->getData()[aidx++] = ctor;
            isOk = true;
        } while(0);

        if(!isOk) { supportFreeObjArray(env, array, aidx); return NULL; }
    }
    return array;
}

jclass NativeClass_GetDeclaringClass0(FNIEnv *env, jclass cls) {
    if(cls->isArray() || cls->isPrimitive()) return NULL;
    const char *clsName = cls->getTypeName();
    uint16_t len = strlen(clsName);
    while(len > 0 && clsName[len - 1] != '$') len--;
    if(len > 0 && clsName[len - 1] == '$') return env->findClass(clsName, len - 1);
    return NULL;
}
