
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_system_api.h"

static uint32_t objectCount = 0;

Flint Flint::flintInstance;

FlintExecutionNode::FlintExecutionNode(Flint &flint) : FlintExecution(flint) {
    prev = 0;
    next = 0;
}

FlintExecutionNode::FlintExecutionNode(Flint &flint, uint32_t stackSize) : FlintExecution(flint, stackSize) {
    prev = 0;
    next = 0;
}

void Flint::lock(void) {
    FlintAPI::Thread::lock();
}

void Flint::unlock(void) {
    FlintAPI::Thread::unlock();
}

void *Flint::malloc(uint32_t size) {
    void *ret = FlintAPI::System::malloc(size);
    if(ret == 0) {
        flintInstance.garbageCollection();
        ret = FlintAPI::System::malloc(size);
        if(ret == 0)
            throw (FlintOutOfMemoryError *)"not enough memory to allocate";
    }
    objectCount++;
    return ret;
}

void *Flint::realloc(void *p, uint32_t size) {
    void *ret = FlintAPI::System::realloc(p, size);
    if(ret == 0) {
        flintInstance.garbageCollection();
        ret = FlintAPI::System::realloc(p, size);
        if(ret == 0)
            throw (FlintOutOfMemoryError *)"not enough memory to allocate";
    }
    return ret;
}

void Flint::free(void *p) {
    objectCount--;
    FlintAPI::System::free(p);
}

Flint &Flint::getInstance(void) {
    return flintInstance;
}

Flint::Flint(void) {
    dbg = 0;
    executionList = 0;
    classDataList = 0;
    objectList = 0;
    constClassList = 0;
    constStringList = 0;
    objectSizeToGc = 0;
    constUtf8List = 0;
}

FlintDebugger *Flint::getDebugger(void) const {
    return dbg;
}

void Flint::setDebugger(FlintDebugger *dbg) {
    this->dbg = dbg;
}

void Flint::print(const char *text, uint32_t length, uint8_t coder) {
    if(dbg)
        dbg->print(text, length, coder);
    else
        FlintAPI::System::print(text, length, coder);
}

FlintExecution &Flint::newExecution(void) {
    FlintExecutionNode *newNode = (FlintExecutionNode *)Flint::malloc(sizeof(FlintExecutionNode));
    new (newNode)FlintExecutionNode(*this);
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *newNode;
}

FlintExecution &Flint::newExecution(uint32_t stackSize) {
    FlintExecutionNode *newNode = (FlintExecutionNode *)Flint::malloc(sizeof(FlintExecutionNode));
    new (newNode)FlintExecutionNode(*this, stackSize);
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *newNode;
}

void Flint::freeExecution(FlintExecution &execution) {
    Flint::lock();
    FlintExecutionNode *prev = ((FlintExecutionNode *)&execution)->prev;
    FlintExecutionNode *next = ((FlintExecutionNode *)&execution)->next;
    ((FlintExecutionNode *)&execution)->~FlintExecutionNode();
    Flint::free(&execution);
    if(prev)
        prev->next = next;
    else
        executionList = next;
    if(next)
        next->prev = prev;
    Flint::unlock();
}

FlintObject &Flint::newObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions) {
    objectSizeToGc += size;
    if(objectSizeToGc >= OBJECT_SIZE_TO_GC)
        garbageCollection();
    FlintObject *newNode = (FlintObject *)Flint::malloc(sizeof(FlintObject) + size);
    new (newNode)FlintObject(size, type, dimensions);

    newNode->prev = 0;
    newNode->next = objectList;
    if(objectList)
        objectList->prev = newNode;
    objectList = newNode;

    return *newNode;
}

FlintObject &Flint::newMultiArray(FlintConstUtf8 &typeName, uint8_t dimensions, int32_t *counts) {
    if(dimensions > 1) {
        FlintObject &array = newObject(counts[0] * sizeof(FlintObject *), typeName, dimensions);
        for(uint32_t i = 0; i < counts[0]; i++)
            ((FlintObject **)(array.data))[i] = &newMultiArray(typeName, dimensions - 1, &counts[1]);
        return array;
    }
    else {
        uint8_t atype = FlintObject::isPrimType(typeName);
        uint8_t typeSize = atype ? FlintObject::getPrimitiveTypeSize(atype) : sizeof(FlintObject *);
        FlintObject &array = newObject(typeSize * counts[0], typeName, 1);
        memset(array.data, 0, array.size);
        return array;
    }
}

FlintClass &Flint::newClass(FlintString &typeName) {
    // TODO - Check the existence of type

    /* create new class object */
    FlintClass &classObj = *(FlintClass *)&newObject(sizeof(FlintFieldsData), *(FlintConstUtf8 *)&classClassName);

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)classObj.data;
    new (fields)FlintFieldsData(*this, load(*(FlintConstUtf8 *)&classClassName), false);

    /* set value for name field */
    classObj.setName(&typeName);

    return classObj;
}

FlintClass &Flint::newClass(const char *typeName, uint16_t length) {
    /* create String object to store typeName */
    FlintString &name = newString(typeName, length);

    /* replace '/' to '.' */
    char *text = (char *)name.getText();
    uint8_t coder = name.getCoder();
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
    return newClass(name);
}

FlintClass &Flint::getConstClass(const char *typeName, uint16_t length) {
    for(FlintConstClass *node = constClassList; node != 0; node = node->next) {
        if(node->flintClass.getName().equals(typeName, length))
            return node->flintClass;
    }
    FlintClass &classObj = newClass(typeName, length);
    FlintConstClass *newNode = (FlintConstClass *)Flint::malloc(sizeof(FlintConstClass));
    new (newNode)FlintConstClass(classObj);

    newNode->next = constClassList;
    constClassList = newNode;

    return classObj;
}

FlintClass &Flint::getConstClass(FlintString &str) {
    for(FlintConstClass *node = constClassList; node != 0; node = node->next) {
        if(node->flintClass.getName().equals(str))
            return node->flintClass;
    }
    FlintClass &classObj = newClass(str);
    FlintConstClass *newNode = (FlintConstClass *)Flint::malloc(sizeof(FlintConstClass));
    new (newNode)FlintConstClass(classObj);

    newNode->next = constClassList;
    constClassList = newNode;

    return classObj;
}

FlintString &Flint::newString(uint16_t length, uint8_t coder) {
    /* create new byte array to store string */
    FlintObject &byteArray = newObject(length << (coder ? 1 : 0), *(FlintConstUtf8 *)primTypeConstUtf8List[4], 1);

    /* create new string object */
    FlintString &strObj = *(FlintString *)&newObject(sizeof(FlintFieldsData), *(FlintConstUtf8 *)&stringClassName);

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)strObj.data;
    new (fields)FlintFieldsData(*this, load(*(FlintConstUtf8 *)&stringClassName), false);

    /* set value for value field */
    strObj.setValue(byteArray);

    /* set value for coder field */
    strObj.setCoder(coder);

    return strObj;
}

FlintString &Flint::newString(const char *text, uint16_t size, bool isUtf8) {
    uint32_t index = 0;
    bool isLatin1 = isUtf8 ? FlintString::isLatin1(text) : true;
    uint32_t strLen = isUtf8 ? FlintString::utf8StrLen(text) : size;

    /* create new byte array to store string */
    uint32_t arrayLen = isLatin1 ? strLen : (strLen << 1);
    FlintObject &byteArray = newObject(arrayLen, *(FlintConstUtf8 *)primTypeConstUtf8List[4], 1);
    if(!isUtf8)
        memcpy(byteArray.data, text, strLen);
    else {
        if(isLatin1) {
            while(*text) {
                uint32_t c = FlintString::utf8Decode(text);
                byteArray.data[index] = c;
                text += FlintString::getUtf8DecodeSize(*text);
                index++;
            }
        }
        else while(*text) {
            uint32_t c = FlintString::utf8Decode(text);
            if(c <= 0xFFFFFF)
                ((uint16_t *)byteArray.data)[index] = c;
            else
                throw "Characters are not supported";
            text += FlintString::getUtf8DecodeSize(*text);
            index++;
        }
    }

    /* create new string object */
    FlintString &strObj = *(FlintString *)&newObject(sizeof(FlintFieldsData), *(FlintConstUtf8 *)&stringClassName);

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)strObj.data;
    new (fields)FlintFieldsData(*this, load(*(FlintConstUtf8 *)&stringClassName), false);

    /* set value for value field */
    strObj.setValue(byteArray);

    /* set value for coder field */
    strObj.setCoder(isLatin1 ? 0 : 1);

    return strObj;
}

FlintString &Flint::newString(const char *latin1Str[], uint16_t count) {
    uint16_t index = 0;
    uint16_t length = 0;
    for(uint16_t i = 0; i < count; i++)
        length += strlen(latin1Str[i]);

    /* create new byte array to store string */
    FlintObject &byteArray = newObject(length, *(FlintConstUtf8 *)primTypeConstUtf8List[4], 1);
    for(uint16_t i = 0; i < count; i++) {
        const char *buff = latin1Str[i];
        while(*buff) {
            uint32_t c = FlintString::utf8Decode(buff);
            byteArray.data[index] = c;
            buff += FlintString::getUtf8DecodeSize(*buff);
            index++;
        }
    }

    /* create new string object */
    FlintString &strObj = *(FlintString *)&newObject(sizeof(FlintFieldsData), *(FlintConstUtf8 *)&stringClassName);

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)strObj.data;
    new (fields)FlintFieldsData(*this, load(*(FlintConstUtf8 *)&stringClassName), false);

    /* set value for value field */
    strObj.setValue(byteArray);

    return strObj;
}

FlintString &Flint::getConstString(FlintConstUtf8 &utf8) {
    for(FlintConstString *node = constStringList; node != 0; node = node->next) {
        if(node->flintString.equals(utf8))
            return node->flintString;
    }
    FlintString &strObj = newString(utf8.text, utf8.length, true);
    FlintConstString *newNode = (FlintConstString *)Flint::malloc(sizeof(FlintConstString));
    new (newNode)FlintConstString(strObj);

    newNode->next = constStringList;
    constStringList = newNode;

    return strObj;
}

FlintString &Flint::getConstString(FlintString &str) {
    for(FlintConstString *node = constStringList; node != 0; node = node->next) {
        if(node->flintString.equals(str))
            return node->flintString;
    }
    FlintConstString *newNode = (FlintConstString *)Flint::malloc(sizeof(FlintConstString));
    new (newNode)FlintConstString(str);

    newNode->next = constStringList;
    constStringList = newNode;

    return str;
}

FlintConstUtf8 &Flint::getConstUtf8(const char *text, uint16_t length) {
    uint32_t hash;
    ((uint16_t *)&hash)[0] = length;
    ((uint16_t *)&hash)[1] = Flint_CalcCrc((uint8_t *)text, length);
    Flint::lock();
    for(FlintConstUtf8Node *node = constUtf8List; node != 0; node = node->next) {
        if(CONST_UTF8_HASH(node->value) == hash) {
            if(strncmp(node->value.text, text, length) == 0) {
                Flint::unlock();
                return node->value;
            }
        }
    }

    FlintConstUtf8Node *newNode = (FlintConstUtf8Node *)Flint::malloc(sizeof(FlintConstUtf8Node) + length + 1);
    *(uint16_t *)&newNode->value.length = length;
    *(uint16_t *)&newNode->value.crc = ((uint16_t *)&hash)[1];
    char *textBuff = (char *)newNode->value.text;
    strncpy(textBuff, text, length);
    textBuff[length] = 0;

    newNode->next = constUtf8List;
    constUtf8List = newNode;

    Flint::unlock();

    return newNode->value;
}

FlintThrowable &Flint::newThrowable(FlintString &strObj, FlintConstUtf8 &excpType) {
    /* create new exception object */
    FlintThrowable &obj = *(FlintThrowable *)&newObject(sizeof(FlintFieldsData), excpType);

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)obj.data;
    new (fields)FlintFieldsData(*this, load(excpType), false);

    /* set detailMessage value */
    obj.setDetailMessage(strObj);

    return obj;
}

FlintThrowable &Flint::newArrayStoreException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&arrayStoreExceptionClassName);
}

FlintThrowable &Flint::newArithmeticException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&arithmeticExceptionClassName);
}

FlintThrowable &Flint::newNullPointerException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&nullPtrExcpClassName);
}

FlintThrowable &Flint::newClassNotFoundException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&classNotFoundExceptionClassName);
}

FlintThrowable &Flint::newCloneNotSupportedException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&cloneNotSupportedExceptionClassName);
}

FlintThrowable &Flint::newNegativeArraySizeException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&negativeArraySizeExceptionClassName);
}

FlintThrowable &Flint::newArrayIndexOutOfBoundsException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&arrayIndexOutOfBoundsExceptionClassName);
}

FlintThrowable &Flint::newUnsupportedOperationException(FlintString &strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&unsupportedOperationExceptionClassName);
}

void Flint::clearProtectObjectNew(FlintObject &obj) {
    bool isPrim = FlintObject::isPrimType(obj.type);
    if((obj.dimensions > 1) || (obj.dimensions == 1 && !isPrim)) {
        uint32_t count = obj.size / 4;
        for(uint32_t i = 0; i < count; i++) {
            FlintObject *tmp = ((FlintObject **)obj.data)[i];
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(*tmp);
        }
    }
    else if(!isPrim) {
        FlintFieldsData &fieldData = *(FlintFieldsData *)obj.data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            FlintObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(*tmp);
        }
    }
    obj.clearProtected();
}

void Flint::garbageCollectionProtectObject(FlintObject &obj) {
    bool isPrim = FlintObject::isPrimType(obj.type);
    if((obj.dimensions > 1) || (obj.dimensions == 1 && !isPrim)) {
        uint32_t count = obj.size / 4;
        for(uint32_t i = 0; i < count; i++) {
            FlintObject *tmp = ((FlintObject **)obj.data)[i];
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(*tmp);
        }
    }
    else if(!isPrim) {
        FlintFieldsData &fieldData = *(FlintFieldsData *)obj.data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            FlintObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(*tmp);
        }
    }
    obj.setProtected();
}

void Flint::garbageCollection(void) {
    Flint::lock();
    objectSizeToGc = 0;
    for(FlintConstClass *node = constClassList; node != 0; node = node->next) {
        if(!node->flintClass.getProtected())
            garbageCollectionProtectObject(node->flintClass);
    }
    for(FlintConstString *node = constStringList; node != 0; node = node->next) {
        if(!node->flintString.getProtected())
            garbageCollectionProtectObject(node->flintString);
    }
    for(ClassData *node = classDataList; node != 0; node = node->next) {
        FlintFieldsData *fieldsData = node->staticFieldsData;
        if(fieldsData && fieldsData->fieldsObjCount) {
            for(uint32_t i = 0; i < fieldsData->fieldsObjCount; i++) {
                FlintObject *obj = fieldsData->fieldsObject[i].object;
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(*obj);
            }
        }
    }
    for(FlintExecutionNode *node = executionList; node != 0; node = node->next) {
        for(int32_t i = 0; i <= node->peakSp; i++) {
            if(node->getStackType(i) == STACK_TYPE_OBJECT) {
                FlintObject *obj = (FlintObject *)node->stack[i];
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(*obj);
            }
        }
    }
    for(FlintObject *node = objectList; node != 0;) {
        FlintObject *next = node->next;
        uint8_t prot = node->getProtected();
        if(prot == 0) {
            if(node->prev)
                node->prev->next = node->next;
            else
                objectList = node->next;
            if(node->next)
                node->next->prev = node->prev;

            if(node->dimensions == 0) {
                FlintFieldsData *fields = (FlintFieldsData *)node->data;
                fields->~FlintFieldsData();
            }
            Flint::free(node);
        }
        else if(!(prot & 0x02))
            node->clearProtected();
        node = next;
    }
    Flint::unlock();
}

FlintClassLoader &Flint::load(const char *className, uint16_t length) {
    Flint::lock();
    ClassData *newNode = 0;
    try {
        if(classDataList) {
            uint32_t hash;
            ((uint16_t *)&hash)[0] = length;
            ((uint16_t *)&hash)[1] = Flint_CalcCrc((uint8_t *)className, length);
            for(ClassData *node = classDataList; node != 0; node = node->next) {
                FlintConstUtf8 &name = node->getThisClass();
                if(hash == CONST_UTF8_HASH(name) && strncmp(name.text, className, length) == 0) {
                    Flint::unlock();
                    return *node;
                }
            }
        }
        newNode = (ClassData *)Flint::malloc(sizeof(ClassData));
        newNode->staticFieldsData = 0;
        new (newNode)ClassData(*this, className, length);
        newNode->next = classDataList;
        classDataList = newNode;
        Flint::unlock();
        return *newNode;
    }
    catch(const char *msg) {
        Flint::unlock();
        if(newNode) {
            newNode->~ClassData();
            Flint::free(newNode);
        }
        throw (FlintLoadFileError *)className;
    }
}

FlintClassLoader &Flint::load(const char *className) {
    return load(className, strlen(className));
}

FlintClassLoader &Flint::load(FlintConstUtf8 &className) {
    Flint::lock();
    ClassData *newNode = 0;
    try {
        uint32_t hash = CONST_UTF8_HASH(className);
        for(ClassData *node = classDataList; node != 0; node = node->next) {
            FlintConstUtf8 &name = node->getThisClass();
            if(hash == CONST_UTF8_HASH(name) && strncmp(name.text, className.text, className.length) == 0) {
                Flint::unlock();
                return *node;
            }
        }
        newNode = (ClassData *)Flint::malloc(sizeof(ClassData));
        new (newNode)ClassData(*this, className.text, className.length);
        newNode->next = classDataList;
        classDataList = newNode;
        Flint::unlock();
        return *newNode;
    }
    catch(const char *msg) {
        Flint::unlock();
        if(newNode) {
            newNode->~ClassData();
            Flint::free(newNode);
        }
        throw (FlintLoadFileError *)className.text;
    }
}

FlintFieldsData &Flint::getStaticFields(FlintConstUtf8 &className) const {
    for(ClassData *node = classDataList; node != 0; node = node->next) {
        if(className == node->getThisClass())
            return *node->staticFieldsData;
    }
    return *(FlintFieldsData *)0;
}

void Flint::initStaticField(ClassData &classData) {
    FlintFieldsData *fieldsData = (FlintFieldsData *)Flint::malloc(sizeof(FlintFieldsData));
    new (fieldsData)FlintFieldsData(*this, classData, true);
    classData.staticFieldsData = fieldsData;
}

FlintMethodInfo &Flint::findMethod(FlintConstMethod &constMethod) {
    FlintClassLoader *loader = &load(constMethod.className);
    while(loader) {
        FlintMethodInfo *methodInfo = &loader->getMethodInfo(constMethod.nameAndType);
        if(methodInfo) {
            if((methodInfo->accessFlag & METHOD_BRIDGE) != METHOD_BRIDGE)
                return *methodInfo;
        }
        FlintConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &load(loader->getSuperClass()) : (FlintClassLoader *)0;
    }
    throw "can't find the method";
}

bool Flint::isInstanceof(FlintObject *obj, const char *typeName, uint16_t length) {
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
        FlintConstUtf8 *objType = &obj->type;
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

void Flint::runToMain(const char *mainClass) {
    newExecution().run(load(mainClass).getMainMethodInfo());
}

void Flint::runToMain(const char *mainClass, uint32_t stackSize) {
    newExecution(stackSize).run(load(mainClass).getMainMethodInfo());
}

bool Flint::isRunning(void) const {
    return executionList ? true : false;
}

void Flint::terminateRequest(void) {
    for(FlintExecutionNode *node = executionList; node != 0; node = node->next)
        node->terminateRequest();
}

void Flint::terminate(void) {
    terminateRequest();
    while(isRunning())
        FlintAPI::Thread::yield();
}

void Flint::clearAllStaticFields(void) {
    for(ClassData *node = classDataList; node != 0; node = node->next)
        node->clearStaticFields();
}

void Flint::freeAllObject(void) {
    Flint::lock();
    for(FlintConstClass *node = constClassList; node != 0;) {
        FlintConstClass *next = node->next;
        Flint::free(node);
        node = next;
    }
    for(FlintConstString *node = constStringList; node != 0;) {
        FlintConstString *next = node->next;
        Flint::free(node);
        node = next;
    }
    for(FlintObject *node = objectList; node != 0;) {
        FlintObject *next = node->next;
        if(node->dimensions == 0) {
            FlintFieldsData *fields = (FlintFieldsData *)node->data;
            fields->~FlintFieldsData();
        }
        Flint::free(node);
        node = next;
    }
    constClassList = 0;
    constStringList = 0;
    objectList = 0;
    objectSizeToGc = 0;
    Flint::unlock();
}

void Flint::freeAllExecution(void) {
    Flint::lock();
    for(FlintExecutionNode *node = executionList; node != 0;) {
        FlintExecutionNode *next = node->next;
        node->~FlintExecutionNode();
        Flint::free(node);
        node = next;
    }
    executionList = 0;
    Flint::unlock();
}

void Flint::freeAllClassLoader(void) {
    Flint::lock();
    for(ClassData *node = classDataList; node != 0;) {
        ClassData *next = node->next;
        node->~ClassData();
        Flint::free(node);
        node = next;
    }
    classDataList = 0;
    Flint::unlock();
}

void Flint::freeAllConstUtf8(void) {
    Flint::lock();
    for(FlintConstUtf8Node *node = constUtf8List; node != 0;) {
        FlintConstUtf8Node *next = node->next;
        Flint::free(node);
        node = next;
    }
    constUtf8List = 0;
    Flint::unlock();
}

void Flint::freeAll(void) {
    freeAllObject();
    freeAllExecution();
    freeAllClassLoader();
    freeAllConstUtf8();
}
