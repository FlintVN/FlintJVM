
#include <new>
#include <string.h>
#include <stdlib.h>
#include "flint.h"
#include "flint_system_api.h"
#include "flint_fields_data.h"

FMutex Flint::flintLock;
FDbg *Flint::dbg = NULL;
FDict<ClassLoader> Flint::loaders;
FDict<JClassDictNode> Flint::classes;
FDict<Utf8DictNode> Flint::utf8s;
FDict<JStringDictNode> Flint::constStr;
FList<FExec> Flint::execs;
FList<JObject> Flint::objs;

JClass *Flint::classOfClass = NULL;

uint32_t Flint::heapCount = 0;
uint32_t Flint::objectCountToGc = 0;
void *Flint::heapStart = (void *)0xFFFFFFFF;
void *Flint::headEnd = (void *)0x00;

alignas(4) static const char outOfMemoryErrorTypeName[] = "java/lang/OutOfMemoryError";

static uint32_t getDimensions(const char *typeName) {
    const char *text = typeName;
    while(*text == '[') text++;
    return (uint32_t)(text - typeName);
}

static bool isPrimitiveTypes(const char *typeName) {
    if(typeName[1] == 0) switch(typeName[0]) {
        case 'Z':
        case 'C':
        case 'F':
        case 'D':
        case 'B':
        case 'S':
        case 'I':
        case 'J':
        case 'V': return true;
        default:  return false;
    }
    return false;
}

static int32_t compareArrayClassName(const char *clsName, uint32_t dimensions, const char *arrayClsName) {
    for(uint8_t i = 0; i < dimensions; i++)
        if('[' != arrayClsName[i]) return (uint8_t)'[' - (uint8_t)arrayClsName[i];
    arrayClsName += dimensions;
    bool isObjectType = !isPrimitiveTypes(clsName) && clsName[0] != '[';
    if(isObjectType) {
        if('L' != *arrayClsName) return (uint8_t)'L' - (uint8_t)*arrayClsName;
        arrayClsName++;
    }
    while(*clsName && *arrayClsName) {
        if(*clsName != *arrayClsName) return (uint8_t)*clsName - (uint8_t)*arrayClsName;
        clsName++;
        arrayClsName++;
    }
    if(isObjectType) if(*clsName == 0) return (uint8_t)';' - (uint8_t)*arrayClsName;
    return 0;
}

void Flint::updateHeapRegion(void *p) {
    if(p < heapStart)
        heapStart = p;
    if(p > headEnd)
        headEnd = p;
}

void Flint::resetHeapRegion(void) {
    heapStart = (void *)0xFFFFFFFF;
    headEnd = (void *)0x00;
}

bool Flint::isHeapPointer(void *p) {
    return (((uint32_t)p & 0x03) == 0) && (heapStart <= p) && (p <= headEnd);
}

void *Flint::malloc(FExec *ctx, uint32_t size) {
    if(++objectCountToGc >= OBJECT_COUNT_TO_GC)
        gc();
    void *p = FlintAPI::System::malloc(size);
    if(p == NULL) {
        gc();
        p = FlintAPI::System::malloc(size);
    }
    if(p == NULL) {
        if(ctx != NULL) {
            JClass *excpCls = Flint::findClass(NULL, outOfMemoryErrorTypeName);
            if(excpCls != NULL)
                ctx->throwNew(excpCls);
            else
                ctx->excp = (JThrowable *)((uint32_t)outOfMemoryErrorTypeName | 0x01);
        }
    }
    else {
        updateHeapRegion(p);
        heapCount++;
    }
    return p;
}

void *Flint::realloc(FExec *ctx, void *p, uint32_t size) {
    p = FlintAPI::System::realloc(p, size);
    if(p == NULL) {
        gc();
        p = FlintAPI::System::malloc(size);
    }
    if(p == NULL) {
        if(ctx != NULL) {
            JClass *excpCls = Flint::findClass(NULL, outOfMemoryErrorTypeName);
            if(excpCls != NULL)
                ctx->throwNew(excpCls);
            else
                ctx->excp = (JThrowable *)((uint32_t)outOfMemoryErrorTypeName | 0x01);
        }
    }
    else
        updateHeapRegion(p);
    return p;
}

void Flint::free(void *p) {
    ::free(p);
    heapCount--;
}

void Flint::lock(void) {
    flintLock.lock();
}

void Flint::unlock(void) {
    flintLock.unlock();
}

FDbg *Flint::getDebugger(void) {
    return Flint::dbg;
}

void Flint::setDebugger(FDbg *dbg) {
    Flint::dbg = dbg;
}

void Flint::print(const char *buff, uint32_t length, uint8_t coder) {
    if(dbg)
        dbg->print(buff, length, coder);
    else
        FlintAPI::System::print(buff, length, coder);
}

void Flint::print(int64_t num) {
    char buff[22] = {0};
    int8_t index = sizeof(buff) - 1;
    bool isNagative = num < 0;
    do {
        buff[--index] = (num % 10) + '0';
        num /= 10;
    } while(num);
    if(isNagative)
        buff[--index] = '-';
    print(&buff[index], sizeof(buff) - index - 1, 0);
}

void Flint::print(const char *ascii) {
    print(ascii, strlen(ascii), 0);
}

void Flint::print(JString *str) {
    print(str->getAscii(), str->getLength(), str->getCoder());
}

void Flint::println(int64_t num) {
    print(num);
    print("\n", 1, 0);
}

void Flint::println(const char *ascii) {
    print(ascii, strlen(ascii), 0);
    print("\n", 1, 0);
}

void Flint::println(JString *str) {
    print(str->getAscii(), str->getLength(), str->getCoder());
    print("\n", 1, 0);
}

const char *Flint::getUtf8(FExec *ctx, const char *utf8, uint16_t length) {
    lock();

    Utf8DictNode *utf8Node = utf8s.find(utf8, length);
    if(utf8Node != NULL) { unlock(); return utf8Node->getValue(); }

    uint16_t len = strlen(utf8);
    len = (len < length) ? len : length;
    utf8Node = (Utf8DictNode *)Flint::malloc(ctx, sizeof(Utf8DictNode) + len + 1);
    if(utf8Node == NULL) { unlock(); return NULL; }
    new (utf8Node)Utf8DictNode(utf8, len);
    utf8s.add(utf8Node);

    unlock();
    return utf8Node->getValue();
}

const char *Flint::getArrayClassName(FExec *ctx, const char *clsName, uint8_t dimensions) {
    lock();

    uint32_t hash = 0;
    bool isObjectType = !isPrimitiveTypes(clsName) && clsName[0] != '[';
    Utf8DictNode *utf8Node = NULL;
    do {
        if(utf8s.root == NULL) break;
        for(uint8_t i = 0; i < dimensions; i++) hash = Hash("[", 1, hash);
        if(isObjectType) hash = Hash("L", 1, hash);
        hash = Hash(clsName, 0, hash);
        if(isObjectType) hash = Hash(";", 1, hash);
        Utf8DictNode *node = (Utf8DictNode *)utf8s.root;
        while(node) {
            int32_t cmp = hash - node->getHashKey();
            if(cmp == 0) cmp = compareArrayClassName(clsName, dimensions, node->value);
            if(cmp == 0) { utf8Node = node; break; }
            else if(cmp < 0) node = (Utf8DictNode *)node->left;
            else node = (Utf8DictNode *)node->right;
        }
    } while(false);
    if(utf8Node != NULL) { unlock(); return utf8Node->getValue(); }

    uint32_t len = strlen(clsName);
    utf8Node = (Utf8DictNode *)Flint::malloc(ctx, sizeof(Utf8DictNode) + dimensions + len + 1 + (isObjectType ? 2 : 0));
    if(utf8Node == NULL) { unlock(); return NULL; }
    new (utf8Node)Utf8DictNode();
    utf8Node->hash = hash;
    char *txt = utf8Node->value;
    while(dimensions--) *txt++ = '[';
    if(isObjectType) *txt++ = 'L';
    while(*clsName) *txt++ = *clsName++;
    if(isObjectType) *txt++ = ';';
    *txt = 0;
    utf8s.add(utf8Node);

    unlock();
    return utf8Node->getValue();
}

bool Flint::isInstanceof(FExec *ctx, JObject *obj, JClass *type) {
    JClass *objType = (obj->type != NULL) ? obj->type : getClassOfClass(ctx);
    return isAssignableFrom(ctx, objType, type);
}

bool Flint::isAssignableFrom(FExec *ctx, JClass *fromType, JClass *toType) {
    const char *typeName1 = fromType->getTypeName();
    const char *typeName2 = toType->getTypeName();
    uint8_t dim1 = getDimensions(typeName1);
    uint8_t dim2 = getDimensions(typeName2);
    const char *compTypeName1 = &typeName1[dim1];
    const char *compTypeName2 = &typeName2[dim2];

    if(compTypeName1[0] == 'L') compTypeName1++;
    if(compTypeName2[0] == 'L') compTypeName2++;

    if(dim1 >= dim2 && strncmp(compTypeName2, "java/lang/Object", 16) == 0) return true;
    if(dim1 != dim2) return false;
    if(isPrimitiveTypes(compTypeName1) || isPrimitiveTypes(compTypeName2))
        return (compTypeName1[0] == compTypeName2[0]) && (compTypeName1[1] == compTypeName2[1]);

    uint32_t len1 = 0, len2 = 0;
    while(compTypeName1[len1] != 0 && compTypeName1[len1] != ';') len1++;
    while(compTypeName2[len2] != 0 && compTypeName2[len2] != ';') len2++;

    ClassLoader *loader = Flint::findLoader(ctx, compTypeName1, len1);
    while(loader != NULL) {
        if(strncmp(loader->getName(), compTypeName2, len2) == 0) return true;
        if(loader == NULL) return false;
        uint16_t ifCount = loader->getInterfacesCount();
        for(uint32_t i = 0; i < ifCount; i++) {
            if(strncmp(loader->getInterface(i), compTypeName2, len2) == 0)
                return true;
        }
        JClass *super = loader->getSuperClass(ctx);
        if(super == NULL) break;
        loader = super->getClassLoader();
    }
    return false;
}

FExec *Flint::newExecution(FExec *ctx, JThread *onwer, uint32_t stackSize) {
    FExec *newExec = (FExec *)Flint::malloc(ctx, sizeof(FExec) + stackSize);
    if(newExec == NULL) return NULL;
    new (newExec)FExec(onwer, stackSize);
    lock();
    execs.add(newExec);
    unlock();
    return newExec;
}

void Flint::freeExecution(FExec *exec) {
    Flint::lock();
    execs.remove(exec);
    Flint::free(exec);
    Flint::unlock();
}

JObject *Flint::newObject(FExec *ctx, JClass *type) {
    if(type == NULL) return NULL;
    JObject *newObj = (JObject *)Flint::malloc(ctx, sizeof(JObject) + sizeof(FieldsData));
    if(newObj == NULL) return NULL;
    new (newObj)JObject(sizeof(FieldsData), type);

    if(newObj->initFields(ctx, type->getClassLoader()) == false) { Flint::free(newObj); return NULL; }

    Flint::lock();
    objs.add(newObj);
    Flint::unlock();

    return newObj;
}

JObject *Flint::newArray(FExec *ctx, JClass *type, uint32_t count) {
    if(type == NULL) return NULL;
    uint8_t compSz = type->componentSize();
    if(compSz == 0) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/lang/IllegalArgumentException"));
        return NULL;
    }
    JObject *newObj = (JObject *)Flint::malloc(ctx, sizeof(JObject) + compSz * count);
    if(newObj == NULL) return NULL;
    new (newObj)JObject(compSz * count, type);

    Flint::lock();
    objs.add(newObj);
    Flint::unlock();

    return newObj;
}

JObject *Flint::newMultiArray(FExec *ctx, JClass *type, int32_t *counts, uint8_t depth) {
    JObject *array = newArray(ctx, type, *counts);
    if(array == NULL) return NULL;
    const char *compTypeName = &type->getTypeName()[1];
    depth--;
    if(compTypeName[0] == '[' && depth > 0) {
        uint32_t i, length = *counts;
        JObject **objData = ((JObjectArray *)array)->getData();
        JClass *compType = Flint::findClass(ctx, compTypeName);
        if(compType == NULL) { freeObject(array); return NULL; }
        counts++;
        for(i = 0; i < length; i++) {
            JObject *tmp = newMultiArray(ctx, compType, counts, depth);
            if(tmp == NULL) {
                while(i > 0) {
                    i--;
                    freeObject(objData[i]);
                }
                freeObject(array);
                return NULL;
            }
            objData[i] = tmp;
        }
    }
    else
        array->clearData();
    return array;
}

JString *Flint::newString(FExec *ctx, const char *utf8) {
    JString *str = (JString *)newObject(ctx, Flint::findClass(ctx, "java/lang/String"));
    if(str == NULL) return NULL;
    str->setUtf8(ctx, utf8);
    return str;
}

JString *Flint::newAscii(FExec *ctx, const char *format, ...) {
    va_list args;
    va_start(args, format);
    return newAscii(ctx, format, args);
}

JString *Flint::newAscii(FExec *ctx, const char *format, va_list args) {
    JString *str = (JString *)newObject(ctx, Flint::findClass(ctx, "java/lang/String"));
    if(str == NULL) return NULL;
    str->setAscii(ctx, format, args);
    return str;
}

static bool verifyComponentType(FExec *ctx, const char *clsName, uint16_t length) {
    uint16_t start = 0;
    while(start < length && clsName[start] == '[') start++;
    uint16_t end = start;
    while(end < length && clsName[end]) end++;
    int32_t len = end - start;
    bool isVaild = true;
    if(clsName[start] == 'L') {
        start++;
        if(clsName[end - 1] == ';') end--;
        else isVaild = false;
        if(isVaild == true) {
            len = end - start;
            if(len <= 0) isVaild = false;
            /* findLoader checked ClassNotFoundException can return immediately if error */
            else if(Flint::findLoader(ctx, &clsName[start], len) == NULL) return false;
        }
    }
    else if(len == 1) {
        switch(clsName[start]) {
            case 'Z':
            case 'C':
            case 'F':
            case 'D':
            case 'B':
            case 'S':
            case 'I':
            case 'J':
            case 'V':
                break;
            default:
                isVaild = false;
        }
    }
    else isVaild = false;
    if(isVaild == false) {
        JClass *excpCls = Flint::findClass(ctx, "java/lang/ClassNotFoundException");
        ctx->throwNew(excpCls, "%.*s", length, clsName);
    }
    return isVaild;
}

JClass *Flint::newClass(FExec *ctx, const char *clsName, uint16_t length, uint8_t flag) {
    ClassLoader *loader = NULL;
    if(!(flag & 0x01)) {        /* Check primitive flag - if not primitive type */
        if(clsName[0] == '[') {
            if(flag & 0x02)     /* Check verify component type name flag */
                if(verifyComponentType(ctx, clsName, length) == false) return NULL;
            loader = findLoader(ctx, "java/lang/Object");
        }
        else
            loader = findLoader(ctx, clsName, length);
        if(loader == NULL) return NULL;
    }

    JClass *clsOfCls = getClassOfClass(ctx);
    if(clsOfCls == NULL) return NULL;
    ClassLoader *jClsLoader = clsOfCls->getClassLoader();
    if(jClsLoader == NULL) return NULL;

    JClass *cls = (JClass *)Flint::malloc(ctx, JClass::size());
    if(cls == NULL) return NULL;
    /* Make sure clsName string is managed */
    clsName = ((flag & 0x01) || clsName[0] == '[') ? getUtf8(ctx, clsName, length) : loader->getName();
    if(clsName == NULL) return NULL;
    new (cls)JClass(clsName, loader);

    if(cls->initFields(ctx, jClsLoader) == false) { Flint::free(cls); return NULL; }

    objs.add(cls);

    return cls;
}

JClass *Flint::newClassOfArray(FExec *ctx, const char *clsName, uint8_t dimensions) {
    clsName = getArrayClassName(ctx, clsName, dimensions);
    if(clsName == NULL) return NULL;
    return newClass(ctx, clsName);
}

JClass *Flint::newClassOfClass(FExec *ctx) {
    ClassLoader *jClsLoader = findLoader(ctx, "java/lang/Class");
    if(jClsLoader == NULL) return NULL;

    JClass *cls = (JClass *)Flint::malloc(ctx, JClass::size());
    if(cls == NULL) return NULL;
    new (cls)JClass(jClsLoader->getName(), jClsLoader);

    if(cls->initFields(ctx, jClsLoader) == false) { Flint::free(cls); return NULL; }

    objs.add(cls);

    return cls;
}

ClassLoader *Flint::findLoader(FExec *ctx, const char *clsName, uint16_t length) {
    lock();
    ClassLoader *loader = loaders.find(clsName, length);
    if(loader == NULL) {
        loader = ClassLoader::load(ctx, clsName, length);
        if(loader == NULL) {
            unlock();
            if(ctx != NULL) {
                JClass *excpCls = Flint::findClass(NULL, "java/lang/ClassNotFoundException");
                if(excpCls == NULL) {
                    alignas(4) static const char *errMsg = "Cannot load java/lang/ClassNotFoundException";
                    ctx->excp = (JThrowable *)((uint32_t)errMsg | 0x01);
                }
                else
                    ctx->throwNew(excpCls, "%.*s", length, clsName);
            }
            return NULL;
        }
        loaders.add(loader);
    }
    unlock();
    return loader;
}

JClass *Flint::findClass(FExec *ctx, const char *clsName, uint16_t length, bool verify) {
    lock();

    JClassDictNode *clsNode = classes.find(clsName, length);
    if(clsNode != NULL) { unlock(); return clsNode->getClass(); }

    JClass *newCls = newClass(ctx, clsName, length, verify ? 0x02 : 0x00);
    if(newCls == NULL) { unlock(); return NULL; }

    clsNode = (JClassDictNode *)Flint::malloc(ctx, sizeof(JClassDictNode));
    if(clsNode == NULL) { unlock(); freeObject(newCls); return NULL; }
    new (clsNode)JClassDictNode(newCls);
    classes.add(clsNode);

    unlock();
    return newCls;
}

JClass *Flint::findClassOfArray(FExec *ctx, const char *clsName, uint8_t dimensions) {
    lock();

    JClassDictNode *clsNode = NULL;
    do {
        if(classes.root == NULL) break;
        uint32_t hash = 0;
        bool isObjectType = !isPrimitiveTypes(clsName) && clsName[0] != '[';
        for(uint8_t i = 0; i < dimensions; i++) hash = Hash("[", 1, hash);
        if(isObjectType) hash = Hash("L", 1, hash);
        hash = Hash(clsName, 0, hash);
        if(isObjectType) hash = Hash(";", 1, hash);
        JClassDictNode *node = (JClassDictNode *)classes.root;
        while(node) {
            int32_t cmp = hash - node->getHashKey();
            if(cmp == 0) cmp = compareArrayClassName(clsName, dimensions, node->cls->getTypeName());
            if(cmp == 0) { clsNode = node; break; }
            else if(cmp < 0) node = (JClassDictNode *)node->left;
            else node = (JClassDictNode *)node->right;
        }
    } while(false);
    if(clsNode != NULL) { unlock(); return clsNode->getClass(); }

    JClass *newCls = newClassOfArray(ctx, clsName, dimensions);
    if(newCls == NULL) { unlock(); return NULL; }

    clsNode = (JClassDictNode *)Flint::malloc(ctx, sizeof(JClassDictNode));
    if(clsNode == NULL) { unlock(); freeObject(newCls); return NULL; }
    new (clsNode)JClassDictNode(newCls);
    classes.add(clsNode);

    unlock();
    return newCls;
}

JClass *Flint::getPrimitiveClass(FExec *ctx, const char *name, uint16_t length) {
    if(JClass::isPrimitive(name, length) == 0) {
        JClass *excpCls = Flint::findClass(ctx, "java/lang/IllegalArgumentException");
        if(ctx != NULL) ctx->throwNew(excpCls, "primitive type name is invalid");
    }
    lock();

    JClassDictNode *clsNode = classes.find(name, length);
    if(clsNode != NULL) { unlock(); return clsNode->getClass(); }

    JClass *newCls = newClass(ctx, name, length, 0x01);
    if(newCls == NULL) { unlock(); return NULL; }

    clsNode = (JClassDictNode *)Flint::malloc(ctx, sizeof(JClassDictNode));
    if(clsNode == NULL) { unlock(); freeObject(newCls); return NULL; }
    new (clsNode)JClassDictNode(newCls);
    classes.add(clsNode);

    unlock();
    return newCls;
}

JClass *Flint::getClassOfClass(FExec *ctx) {
    if(classOfClass != NULL) return classOfClass;

    /* Reimplement JClass creation instead of using findClass or newClass to avoid infinite recursion */
    do {
        lock();

        JClassDictNode *clsNode = classes.find("java/lang/Class");
        if(clsNode != NULL) { unlock(); classOfClass = clsNode->getClass(); break; }

        JClass *newCls = newClassOfClass(ctx);
        if(newCls == NULL) { unlock(); break; }

        clsNode = (JClassDictNode *)Flint::malloc(ctx, sizeof(JClassDictNode));
        if(clsNode == NULL) { unlock(); freeObject(newCls); break; }
        new (clsNode)JClassDictNode(newCls);
        classes.add(clsNode);

        unlock();
        classOfClass = newCls;
    } while(false);

    return classOfClass;
}

MethodInfo *Flint::findMethod(FExec *ctx, JClass *cls, ConstNameAndType *nameAndType) {
    if(cls == NULL) return NULL;
    ClassLoader *loader = cls->getClassLoader();
    while(loader != NULL) {
        MethodInfo *mtInfo = loader->getMethodInfo(ctx, nameAndType);
        if(mtInfo != NULL) return mtInfo;
        if(ctx->excp != NULL) return NULL;
        JClass *super = loader->getSuperClass(ctx);
        if(super == NULL) break;
        loader = super->getClassLoader();
    }
    if(ctx != NULL && !ctx->hasException())
        ctx->throwNew(Flint::findClass(ctx, "java/lang/NoSuchMethodError"), "%s.%s", cls->getTypeName(), nameAndType->name);
    return NULL;
}

JString *Flint::getConstString(FExec *ctx, const char *utf8) {
    lock();

    JStringDictNode *strNode = constStr.find(utf8);
    if(strNode != NULL) { unlock(); return strNode->getString(); }

    JString *newStr = newString(ctx, utf8);
    if(newStr == NULL) { unlock(); return NULL; }

    strNode = (JStringDictNode *)Flint::malloc(ctx, sizeof(JStringDictNode));
    if(strNode == NULL) { unlock(); freeObject(newStr); return NULL; }
    new (strNode)JStringDictNode(newStr);
    constStr.add(strNode);

    unlock();
    return newStr;
}

JString *Flint::getConstString(FExec *ctx, JString *str) {
    JStringDictNode tmp(str);
    lock();

    JStringDictNode *strNode = constStr.find(&tmp);
    if(strNode != NULL) { unlock(); return strNode->getString(); }

    strNode = (JStringDictNode *)Flint::malloc(ctx, sizeof(JStringDictNode));
    if(strNode == NULL) { unlock(); return NULL; }
    new (strNode)JStringDictNode(str);
    constStr.add(strNode);

    unlock();
    return str;
}

void Flint::clearProtectLevel2(JObject *obj) {
    lock();
    /* This step to ensure all objects are cleared level 2 protection  */
    markObjectRecursion(obj);
    /* Final step to clear all the marks */
    clearMarkRecursion(obj);
    unlock();
}

void Flint::clearMarkRecursion(JObject *obj) {
    obj->clearProtected();
    const char *typeName = obj->getTypeName();
    if(typeName[0] == '[') {
        if(typeName[1] == '[' || typeName[1] == 'L') {
            JObjectArray *array = (JObjectArray *)obj;
            JObject **data = array->getData();
            uint32_t count = array->getLength();
            for(uint32_t i = 0; i < count; i++) {
                if(data[i] && (data[i]->getProtected() & 0x01))
                    clearMarkRecursion(data[i]);
            }
        }
    }
    else {
        FieldsData *fieldData = obj->getFields();
        for(uint16_t i = 0; i < fieldData->fieldsObjCount; i++) {
            JObject *tmp = fieldData->fieldsObj[i].value;
            if(tmp && (tmp->getProtected() & 0x01))
                clearMarkRecursion(tmp);
        }
    }
}

void Flint::markObjectRecursion(JObject *obj) {
    obj->setProtected();
    const char *typeName = obj->getTypeName();
    if(typeName[0] == '[') {
        if(typeName[1] == '[' || typeName[1] == 'L') {
            JObjectArray *array = (JObjectArray *)obj;
            JObject **data = array->getData();
            uint32_t count = array->getLength();
            for(uint32_t i = 0; i < count; i++) {
                if(data[i] && (data[i]->getProtected() & 0x01) == 0)
                    markObjectRecursion(data[i]);
            }
        }
    }
    else {
        FieldsData *fieldData = obj->getFields();
        for(uint16_t i = 0; i < fieldData->fieldsObjCount; i++) {
            JObject *tmp = fieldData->fieldsObj[i].value;
            if(tmp && (tmp->getProtected() & 0x01) == 0)
                markObjectRecursion(tmp);
        }
    }
}

bool Flint::isObject(void *p) {
    if(!isHeapPointer(p) || objs.root == NULL) return false;
    JObject *obj = (JObject *)p;
    if(obj->onwerList != (void *)&objs) return false;
    if(obj->prev == NULL) return p == objs.root;
    if(!isHeapPointer(obj->prev)) return false;
    if(obj->prev->next != p) return false;
    if(obj->next != NULL) {
        if(!isHeapPointer(obj->next)) return false;
        return obj->next->prev == p;
    }
    return true;
}

void Flint::gc(void) {
    Flint::lock();
    objectCountToGc = 0;
    constStr.forEach([](JStringDictNode *item) {
        JString *str = item->getString();
        if((str->getProtected() & 0x01) == 0)
            markObjectRecursion(str);
    });
    execs.forEach([](FExec *exec) {
        if(exec->onwerThread && (exec->onwerThread->getProtected() & 0x01) == 0)
            markObjectRecursion(exec->onwerThread);
        if(exec->excp != NULL && ((uint32_t)exec->excp & 0x01) != 0 && (exec->excp->getProtected() & 0x01) == 0)
            markObjectRecursion(exec->excp);
        int32_t startSp = exec->startSp;
        int32_t endSp = (exec->sp > exec->peakSp) ? exec->sp : exec->peakSp;
        while(startSp >= 3) {
            for(int32_t i = startSp; i <= endSp; i++) {
                if(isObject((void *)exec->stack[i])) {
                    JObject *obj = (JObject *)exec->stack[i];
                    if(obj && (obj->getProtected() & 0x01) == 0)
                        markObjectRecursion(obj);
                }
            }
            endSp = startSp - 4;
            startSp = exec->stack[startSp];
        }
    });
    objs.forEach([](JObject *obj) {
        uint8_t prot = obj->getProtected();
        /* Free object if it is not marked and not a JClass */
        if(prot == 0 && obj->type != NULL) freeObject(obj);
        else if(!(prot & 0x02)) obj->clearProtected();
    });
    Flint::unlock();
}

bool Flint::runToMain(const char *cls) {
    FExec *exec = Flint::newExecution(NULL);
    if(exec == NULL) return false;
    JClass *mainCls = Flint::findClass(NULL, cls);
    if(mainCls == NULL) return false;
    return exec->run(mainCls->getClassLoader()->getMainMethodInfo(NULL));
}

bool Flint::isRunning(void) {
    return (execs.root != NULL) ? true : false;
}

void Flint::stopRequest(void) {
    Flint::lock();
    execs.forEach([](FExec *exec) {
        exec->stopRequest();
    });
    Flint::unlock();
}

void Flint::terminateRequest(void) {
    Flint::lock();
    execs.forEach([](FExec *exec) {
        exec->terminateRequest();
    });
    Flint::unlock();
}

void Flint::terminate(void) {
    terminateRequest();
    while(isRunning())
        FlintAPI::Thread::sleep(1);
}

void Flint::freeObject(JObject *obj) {
    objs.remove(obj);
    if(obj->isArray() == false)
        obj->getFields()->~FieldsData();
    Flint::free(obj);
}

void Flint::freeAllObject(void) {
    Flint::lock();
    classes.forEach([](JClassDictNode *item) { Flint::free(item); });
    classes.clear();
    constStr.forEach([](JStringDictNode *item) { Flint::free(item); });
    constStr.clear();
    objs.forEach([](JObject *obj) { obj->~JObject(); Flint::free(obj); });
    objs.clear();
    objectCountToGc = 0;
    Flint::unlock();
}

void Flint::clearAllStaticFields(void) {
    loaders.forEach([](ClassLoader *item) {
        item->monitorOwnId = 0;
        item->clearStaticFields();
    });
}

void Flint::freeAllExecution(void) {
    Flint::lock();
    execs.forEach([](FExec *exec) {
        execs.remove(exec);
        Flint::free(exec);
    });
    execs.clear();
    Flint::unlock();
}

void Flint::freeAllClassLoader(void) {
    Flint::lock();
    loaders.forEach([](ClassLoader *item) {
        item->~ClassLoader();
        Flint::free(item);
    });
    loaders.clear();
    classOfClass = NULL;
    Flint::unlock();
}

void Flint::freeAllConstUtf8(void) {
    Flint::lock();
    utf8s.forEach([](Utf8DictNode *item) { Flint::free(item); });
    utf8s.clear();
    Flint::unlock();
}

void Flint::freeAll(void) {
    freeAllObject();
    freeAllExecution();
    freeAllClassLoader();
    freeAllConstUtf8();
}

void Flint::reset(void) {
    FlintAPI::System::reset();
}
