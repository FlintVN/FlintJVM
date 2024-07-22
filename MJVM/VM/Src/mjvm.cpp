
#include <new>
#include <string.h>
#include "mjvm.h"
#include "mjvm_system_api.h"

static uint32_t objectCount = 0;

Mjvm Mjvm::mjvmInstance;

MjvmExecutionNode::MjvmExecutionNode(Mjvm &mjvm) : MjvmExecution(mjvm) {
    prev = 0;
    next = 0;
}

MjvmExecutionNode::MjvmExecutionNode(Mjvm &mjvm, uint32_t stackSize) : MjvmExecution(mjvm, stackSize) {
    prev = 0;
    next = 0;
}

void Mjvm::lock(void) {

}

void Mjvm::unlock(void) {

}

void *Mjvm::malloc(uint32_t size) {
    void *ret = MjvmSystem_Malloc(size);
    if(ret == 0) {
        mjvmInstance.garbageCollection();
        ret = MjvmSystem_Malloc(size);
        if(ret == 0)
            throw (MjvmOutOfMemoryError *)"not enough memory to allocate";
    }
    objectCount++;
    return ret;
}

void *Mjvm::realloc(void *p, uint32_t size) {
    void *ret = MjvmSystem_Realloc(p, size);
    if(ret == 0) {
        mjvmInstance.garbageCollection();
        ret = MjvmSystem_Realloc(p, size);
        if(ret == 0)
            throw (MjvmOutOfMemoryError *)"not enough memory to allocate";
    }
    return ret;
}

void Mjvm::free(void *p) {
    objectCount--;
    MjvmSystem_Free(p);
}

Mjvm &Mjvm::getInstance(void) {
    return mjvmInstance;
}

Mjvm::Mjvm(void) {
    dbg = 0;
    executionList = 0;
    classDataList = 0;
    objectList = 0;
    constClassList = 0;
    constStringList = 0;
    objectSizeToGc = 0;
}

MjvmDebugger *Mjvm::getDebugger(void) const {
    return dbg;
}

void Mjvm::setDebugger(MjvmDebugger *dbg) {
    this->dbg = dbg;
}

MjvmExecution &Mjvm::newExecution(void) {
    MjvmExecutionNode *newNode = (MjvmExecutionNode *)Mjvm::malloc(sizeof(MjvmExecutionNode));
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *new (newNode)MjvmExecutionNode(*this);
}

MjvmExecution &Mjvm::newExecution(uint32_t stackSize) {
    MjvmExecutionNode *newNode = (MjvmExecutionNode *)Mjvm::malloc(sizeof(MjvmExecutionNode));
    new (newNode)MjvmExecutionNode(*this, stackSize);
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *newNode;
}

MjvmObject *Mjvm::newObject(uint32_t size, MjvmConstUtf8 &type, uint8_t dimensions) {
    objectSizeToGc += size;
    if(objectSizeToGc >= OBJECT_SIZE_TO_GC)
        garbageCollection();
    MjvmObject *newNode = (MjvmObject *)Mjvm::malloc(sizeof(MjvmObject) + size);
    new (newNode)MjvmObject(size, type, dimensions);

    newNode->prev = 0;
    newNode->next = objectList;
    if(objectList)
        objectList->prev = newNode;
    objectList = newNode;

    return newNode;
}

MjvmObject *Mjvm::newMultiArray(MjvmConstUtf8 &typeName, uint8_t dimensions, int32_t *counts) {
    if(dimensions > 1) {
        MjvmObject *array = newObject(counts[0] * sizeof(MjvmObject *), typeName, dimensions);
        for(uint32_t i = 0; i < counts[0]; i++)
            ((MjvmObject **)(array->data))[i] = newMultiArray(typeName, dimensions - 1, &counts[1]);
        return array;
    }
    else {
        uint8_t atype = MjvmObject::isPrimType(typeName);
        uint8_t typeSize = atype ? MjvmObject::getPrimitiveTypeSize(atype) : sizeof(MjvmObject *);
        MjvmObject *array = newObject(typeSize * counts[0], typeName, 1);
        memset(array->data, 0, array->size);
        return array;
    }
}

MjvmClass *Mjvm::newClass(MjvmString &typeName) {
    // TODO - Check the existence of type

    /* create new class object */
    MjvmObject *classObj = newObject(sizeof(MjvmFieldsData), *(MjvmConstUtf8 *)&classClassName);

    /* init field data */
    MjvmFieldsData *fields = (MjvmFieldsData *)classObj->data;
    new (fields)MjvmFieldsData(*this, load(*(MjvmConstUtf8 *)&classClassName), false);

    /* set value for name field */
    fields->getFieldObject(*(MjvmConstNameAndType *)stringNameFieldName).object = &typeName;

    return (MjvmClass *)classObj;
}

MjvmClass *Mjvm::newClass(const char *typeName, uint16_t length) {
    /* create String object to store typeName */
    MjvmString *name = newString(typeName, length);

    /* replace '/' to '.' */
    char *text = (char *)name->getText();
    uint8_t coder = name->getCoder();
    if(coder == 0) {
        for(uint32_t i = 0; i < length; i++) {
            if(text[i] == '/')
                text[i] = '.';
        }
    }
    else {
        uint16_t *buff = (uint16_t *)text;
        for(uint32_t i = 0; i < length; i++) {
            if(buff[i] == '/')
                buff[i] = '.';
        }
    }
    return newClass(*name);
}

MjvmClass *Mjvm::getConstClass(const char *typeName, uint16_t length) {
    for(MjvmConstClass *node = constClassList; node != 0; node = node->next) {
        if(node->mjvmClass.getName().equals(typeName, length))
            return &node->mjvmClass;
    }
    MjvmClass *classObj = newClass(typeName, length);
    MjvmConstClass *newNode = (MjvmConstClass *)Mjvm::malloc(sizeof(MjvmConstClass));
    new (newNode)MjvmConstClass(*classObj);

    newNode->next = constClassList;
    constClassList = newNode;

    return classObj;
}

MjvmClass *Mjvm::getConstClass(MjvmString &str) {
    for(MjvmConstClass *node = constClassList; node != 0; node = node->next) {
        if(node->mjvmClass.getName().equals(str))
            return &node->mjvmClass;
    }
    MjvmClass *classObj = newClass(str);
    MjvmConstClass *newNode = (MjvmConstClass *)Mjvm::malloc(sizeof(MjvmConstClass));
    new (newNode)MjvmConstClass(*classObj);

    newNode->next = constClassList;
    constClassList = newNode;

    return classObj;
}

MjvmString *Mjvm::newString(uint16_t length, uint8_t coder) {
    /* create new byte array to store string */
    MjvmObject *byteArray = newObject(length << (coder ? 1 : 0), *(MjvmConstUtf8 *)primTypeConstUtf8List[4], 1);

    /* create new string object */
    MjvmObject *strObj = newObject(sizeof(MjvmFieldsData), *(MjvmConstUtf8 *)&stringClassName);

    /* init field data */
    MjvmFieldsData *fields = (MjvmFieldsData *)strObj->data;
    new (fields)MjvmFieldsData(*this, load(*(MjvmConstUtf8 *)&stringClassName), false);

    /* set value for value field */
    fields->getFieldObject(*(MjvmConstNameAndType *)stringValueFieldName).object = byteArray;

    /* set value for coder field */
    fields->getFieldData32(*(MjvmConstNameAndType *)stringCoderFieldName).value = coder;

    return (MjvmString *)strObj;
}

MjvmString *Mjvm::newString(const char *text, uint16_t size, bool isUtf8) {
    uint32_t index = 0;
    bool isLatin1 = isUtf8 ? MjvmString::isLatin1(text) : true;
    uint32_t strLen = isUtf8 ? MjvmString::utf8StrLen(text) : size;

    /* create new byte array to store string */
    uint32_t arrayLen = isLatin1 ? strLen : (strLen << 1);
    MjvmObject *byteArray = newObject(arrayLen, *(MjvmConstUtf8 *)primTypeConstUtf8List[4], 1);
    if(!isUtf8)
        memcpy(byteArray->data, text, strLen);
    else {
        if(isLatin1) {
            while(*text) {
                uint32_t c = MjvmString::utf8Decode(text);
                byteArray->data[index] = c;
                text += MjvmString::getUtf8DecodeSize(*text);
                index++;
            }
        }
        else while(*text) {
            uint32_t c = MjvmString::utf8Decode(text);
            if(c <= 0xFFFFFF)
                ((uint16_t *)byteArray->data)[index] = c;
            else
                throw "Characters are not supported";
            text += MjvmString::getUtf8DecodeSize(*text);
            index++;
        }
    }

    /* create new string object */
    MjvmObject *strObj = newObject(sizeof(MjvmFieldsData), *(MjvmConstUtf8 *)&stringClassName);

    /* init field data */
    MjvmFieldsData *fields = (MjvmFieldsData *)strObj->data;
    new (fields)MjvmFieldsData(*this, load(*(MjvmConstUtf8 *)&stringClassName), false);

    /* set value for value field */
    fields->getFieldObject(*(MjvmConstNameAndType *)stringValueFieldName).object = byteArray;

    /* set value for coder field */
    fields->getFieldData32(*(MjvmConstNameAndType *)stringCoderFieldName).value = isLatin1 ? 0 : 1;

    return (MjvmString *)strObj;
}

MjvmString *Mjvm::newString(const char *latin1Str[], uint16_t count) {
    uint16_t index = 0;
    uint16_t length = 0;
    for(uint16_t i = 0; i < count; i++)
        length += strlen(latin1Str[i]);

    /* create new byte array to store string */
    MjvmObject *byteArray = newObject(length, *(MjvmConstUtf8 *)primTypeConstUtf8List[4], 1);
    for(uint16_t i = 0; i < count; i++) {
        const char *buff = latin1Str[i];
        while(*buff) {
            uint32_t c = MjvmString::utf8Decode(buff);
            byteArray->data[index] = c;
            buff += MjvmString::getUtf8DecodeSize(*buff);
            index++;
        }
    }

    /* create new string object */
    MjvmObject *strObj = newObject(sizeof(MjvmFieldsData), *(MjvmConstUtf8 *)&stringClassName);

    /* init field data */
    MjvmFieldsData *fields = (MjvmFieldsData *)strObj->data;
    new (fields)MjvmFieldsData(*this, load(*(MjvmConstUtf8 *)&stringClassName), false);

    /* set value for value field */
    fields->getFieldObject(*(MjvmConstNameAndType *)stringValueFieldName).object = byteArray;

    return (MjvmString *)strObj;
}

MjvmString *Mjvm::getConstString(MjvmConstUtf8 &utf8) {
    for(MjvmConstString *node = constStringList; node != 0; node = node->next) {
        if(node->mjvmString.equals(utf8))
            return &node->mjvmString;
    }
    MjvmString *strObj = newString(utf8.text, utf8.length, true);
    MjvmConstString *newNode = (MjvmConstString *)Mjvm::malloc(sizeof(MjvmConstString));
    new (newNode)MjvmConstString(*strObj);

    newNode->next = constStringList;
    constStringList = newNode;

    return strObj;
}

MjvmString *Mjvm::getConstString(MjvmString &str) {
    for(MjvmConstString *node = constStringList; node != 0; node = node->next) {
        if(node->mjvmString.equals(str))
            return &node->mjvmString;
    }
    MjvmConstString *newNode = (MjvmConstString *)Mjvm::malloc(sizeof(MjvmConstString));
    new (newNode)MjvmConstString(str);

    newNode->next = constStringList;
    constStringList = newNode;

    return &str;
}

MjvmThrowable *Mjvm::newThrowable(MjvmString *strObj, MjvmConstUtf8 &excpType) {
    /* create new exception object */
    MjvmObject *obj = newObject(sizeof(MjvmFieldsData), excpType);

    /* init field data */
    MjvmFieldsData *fields = (MjvmFieldsData *)obj->data;
    new (fields)MjvmFieldsData(*this, load(excpType), false);

    /* set detailMessage value */
    fields->getFieldObject(*(MjvmConstNameAndType *)exceptionDetailMessageFieldName).object = strObj;

    return (MjvmThrowable *)obj;
}

MjvmThrowable *Mjvm::newArrayStoreException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&arrayStoreExceptionClassName);
}

MjvmThrowable *Mjvm::newArithmeticException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&arithmeticExceptionClassName);
}

MjvmThrowable *Mjvm::newNullPointerException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&nullPtrExcpClassName);
}

MjvmThrowable *Mjvm::newClassNotFoundException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&classNotFoundExceptionClassName);
}

MjvmThrowable *Mjvm::newCloneNotSupportedException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&cloneNotSupportedExceptionClassName);
}

MjvmThrowable *Mjvm::newNegativeArraySizeException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&negativeArraySizeExceptionClassName);
}

MjvmThrowable *Mjvm::newArrayIndexOutOfBoundsException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&arrayIndexOutOfBoundsExceptionClassName);
}

MjvmThrowable *Mjvm::newUnsupportedOperationException(MjvmString *strObj) {
    return newThrowable(strObj, *(MjvmConstUtf8 *)&unsupportedOperationExceptionClassName);
}

void Mjvm::freeAllObject(void) {
    for(MjvmConstClass *node = constClassList; node != 0;) {
        MjvmConstClass *next = node->next;
        Mjvm::free(node);
        node = next;
    }
    for(MjvmConstString *node = constStringList; node != 0;) {
        MjvmConstString *next = node->next;
        Mjvm::free(node);
        node = next;
    }
    for(MjvmObject *node = objectList; node != 0;) {
        MjvmObject *next = node->next;
        if(node->dimensions == 0) {
            MjvmFieldsData *fields = (MjvmFieldsData *)node->data;
            fields->~MjvmFieldsData();
        }
        Mjvm::free(node);
        node = next;
    }
    objectList = 0;
}

void Mjvm::clearProtectObjectNew(MjvmObject *obj) {
    bool isPrim = MjvmObject::isPrimType(obj->type);
    if((obj->dimensions > 1) || (obj->dimensions == 1 && !isPrim)) {
        uint32_t count = obj->size / 4;
        for(uint32_t i = 0; i < count; i++) {
            MjvmObject *tmp = ((MjvmObject **)obj->data)[i];
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(tmp);
        }
    }
    else if(!isPrim) {
        MjvmFieldsData &fieldData = *(MjvmFieldsData *)obj->data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            MjvmObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(tmp);
        }
    }
    obj->clearProtected();
}

void Mjvm::garbageCollectionProtectObject(MjvmObject *obj) {
    bool isPrim = MjvmObject::isPrimType(obj->type);
    if((obj->dimensions > 1) || (obj->dimensions == 1 && !isPrim)) {
        uint32_t count = obj->size / 4;
        for(uint32_t i = 0; i < count; i++) {
            MjvmObject *tmp = ((MjvmObject **)obj->data)[i];
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(tmp);
        }
    }
    else if(!isPrim) {
        MjvmFieldsData &fieldData = *(MjvmFieldsData *)obj->data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            MjvmObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(tmp);
        }
    }
    obj->setProtected();
}

void Mjvm::garbageCollection(void) {
    Mjvm::lock();
    objectSizeToGc = 0;
    for(MjvmConstClass *node = constClassList; node != 0; node = node->next) {
        if(!node->mjvmClass.getProtected())
            garbageCollectionProtectObject(&node->mjvmClass);
    }
    for(MjvmConstString *node = constStringList; node != 0; node = node->next) {
        if(!node->mjvmString.getProtected())
            garbageCollectionProtectObject(&node->mjvmString);
    }
    for(ClassData *node = classDataList; node != 0; node = node->next) {
        MjvmFieldsData *fieldsData = node->staticFiledsData;
        if(fieldsData && fieldsData->fieldsObjCount) {
            for(uint32_t i = 0; i < fieldsData->fieldsObjCount; i++) {
                MjvmObject *obj = fieldsData->fieldsObject[i].object;
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(obj);
            }
        }
    }
    for(MjvmExecutionNode *node = executionList; node != 0; node = node->next) {
        for(int32_t i = 0; i <= node->peakSp; i++) {
            if(node->getStackType(i) == STACK_TYPE_OBJECT) {
                MjvmObject *obj = (MjvmObject *)node->stack[i];
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(obj);
            }
        }
    }
    for(MjvmObject *node = objectList; node != 0;) {
        MjvmObject *next = node->next;
        uint8_t prot = node->getProtected();
        if(prot == 0) {
            if(node->prev)
                node->prev->next = node->next;
            else
                objectList = node->next;
            if(node->next)
                node->next->prev = node->prev;

            if(node->dimensions == 0) {
                MjvmFieldsData *fields = (MjvmFieldsData *)node->data;
                fields->~MjvmFieldsData();
            }
            Mjvm::free(node);
        }
        else if(!(prot & 0x02))
            node->clearProtected();
        node = next;
    }
    Mjvm::unlock();
}

MjvmClassLoader &Mjvm::load(const char *className, uint16_t length) {
    Mjvm::lock();
    ClassData *newNode = 0;
    try {
        if(classDataList) {
            uint32_t hash;
            ((uint16_t *)&hash)[0] = length;
            ((uint16_t *)&hash)[1] = Mjvm_CalcCrc((uint8_t *)className, length);
            for(ClassData *node = classDataList; node != 0; node = node->next) {
                MjvmConstUtf8 &name = node->getThisClass();
                if(name.length == length && strncmp(name.text, className, length) == 0)
                    return *node;
            }
        }
        newNode = (ClassData *)Mjvm::malloc(sizeof(ClassData));
        memset((void *)newNode, 0, sizeof(ClassData));
        new (newNode)ClassData(className, length);
        newNode->next = classDataList;
        classDataList = newNode;
        Mjvm::unlock();
        return *newNode;
    }
    catch(const char *msg) {
        Mjvm::unlock();
        if(newNode) {
            newNode->~ClassData();
            Mjvm::free(newNode);
        }
        throw (MjvmLoadFileError *)className;
    }
}

MjvmClassLoader &Mjvm::load(const char *className) {
    return load(className, strlen(className));
}

MjvmClassLoader &Mjvm::load(MjvmConstUtf8 &className) {
    Mjvm::lock();
    ClassData *newNode = 0;
    try {
        uint32_t hash = CONST_UTF8_HASH(className);
        for(ClassData *node = classDataList; node != 0; node = node->next) {
            MjvmConstUtf8 &name = node->getThisClass();
            if(hash == CONST_UTF8_HASH(name) && strncmp(name.text, className.text, className.length) == 0)
                return *node;
        }
        newNode = (ClassData *)Mjvm::malloc(sizeof(ClassData));
        memset((void *)newNode, 0, sizeof(ClassData));
        new (newNode)ClassData(className.text, className.length);
        newNode->next = classDataList;
        classDataList = newNode;
        Mjvm::unlock();
        return *newNode;
    }
    catch(const char *msg) {
        Mjvm::unlock();
        if(newNode) {
            newNode->~ClassData();
            Mjvm::free(newNode);
        }
        throw (MjvmLoadFileError *)className.text;
    }
}

MjvmFieldsData &Mjvm::getStaticFields(MjvmConstUtf8 &className) const {
    for(ClassData *node = classDataList; node != 0; node = node->next) {
        if(className == node->getThisClass())
            return *node->staticFiledsData;
    }
    return *(MjvmFieldsData *)0;
}

void Mjvm::initStaticField(ClassData &classData) {
    MjvmFieldsData *fieldsData = (MjvmFieldsData *)Mjvm::malloc(sizeof(MjvmFieldsData));
    new (fieldsData)MjvmFieldsData(*this, classData, true);
    classData.staticFiledsData = fieldsData;
}

MjvmMethodInfo &Mjvm::findMethod(MjvmConstMethod &constMethod) {
    MjvmClassLoader *loader = &load(constMethod.className);
    while(loader) {
        MjvmMethodInfo *methodInfo = &loader->getMethodInfo(constMethod.nameAndType);
        if(methodInfo) {
            if((methodInfo->accessFlag & METHOD_BRIDGE) != METHOD_BRIDGE)
                return *methodInfo;
        }
        MjvmConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &load(loader->getSuperClass()) : (MjvmClassLoader *)0;
    }
    throw "can't find the method";
}

bool Mjvm::isInstanceof(MjvmObject *obj, const char *typeName, uint16_t length) {
    const char *text = typeName;
    while(*text == '[')
        text++;
    uint32_t dimensions = text - typeName;
    uint32_t len = length - dimensions;
    if(*text == 'L') {
        text++;
        len -= 2;
    }
    if(len == 16 && obj->dimensions >= dimensions) {
        bool isEquals = true;
        const char *text2 = objectClassName.text;
        for(uint32_t i = 0; i < len; i++) {
            if(text[i] == text2[i])
                continue;
            else if((text[i] == '.' && text2[i] == '/') || (text[i] == '/' && text2[i] == '.'))
                continue;
            isEquals = false;
            break;
        }
        if(isEquals)
            return true;
    }
    if(dimensions != obj->dimensions)
        return false;
    else {
        MjvmConstUtf8 *objType = &obj->type;
        while(1) {
            if(len == objType->length) {
                bool isEquals = true;
                const char *text2 = objType->text;
                for(uint32_t i = 0; i < len; i++) {
                    if(text[i] == text2[i])
                        continue;
                    else if((text[i] == '.' && text2[i] == '/') || (text[i] == '/' && text2[i] == '.'))
                        continue;
                    isEquals = false;
                    break;
                }
                if(isEquals)
                    return true;
            }
            objType = &load(*objType).getSuperClass();
            if(objType == 0)
                return false;
        }
    }
}

void Mjvm::terminateAll(void) {
    for(MjvmExecutionNode *node = executionList; node != 0; node = node->next)
        node->terminateRequest();
    for(MjvmExecutionNode *node = executionList; node != 0;) {
        MjvmExecutionNode *next = node->next;
        while(node->isRunning());
        node->~MjvmExecution();
        Mjvm::free(node);
        node = next;
    }
}
