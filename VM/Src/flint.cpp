
#include <new>
#include <string.h>
#include "flint.h"

static uint32_t objectCount = 0;

FlintAPI::Thread::LockHandle *Flint::flintLockHandle = FlintAPI::Thread::createLockHandle();

Flint Flint::flintInstance;

FlintExecutionNode::FlintExecutionNode(Flint &flint, FlintJavaThread *onwerThread) : FlintExecution(flint, onwerThread) {
    prev = 0;
    next = 0;
}

FlintExecutionNode::FlintExecutionNode(Flint &flint, FlintJavaThread *onwerThread, uint32_t stackSize) : FlintExecution(flint, onwerThread, stackSize) {
    prev = 0;
    next = 0;
}

void Flint::lock(void) {
    FlintAPI::Thread::lock(flintLockHandle);
}

void Flint::unlock(void) {
    FlintAPI::Thread::unlock(flintLockHandle);
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

FlintExecution &Flint::newExecution(FlintJavaThread *onwerThread) {
    FlintExecutionNode *newNode = (FlintExecutionNode *)Flint::malloc(sizeof(FlintExecutionNode));
    new (newNode)FlintExecutionNode(*this, onwerThread);
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *newNode;
}

FlintExecution &Flint::newExecution(FlintJavaThread *onwerThread, uint32_t stackSize) {
    FlintExecutionNode *newNode = (FlintExecutionNode *)Flint::malloc(sizeof(FlintExecutionNode));
    new (newNode)FlintExecutionNode(*this, onwerThread, stackSize);
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return *newNode;
}

FlintExecution *Flint::getExcutionByThread(FlintJavaThread &thread) const {
    for(FlintExecutionNode *node = executionList; node != 0; node = node->next) {
        if(node->onwerThread == &thread)
            return node;
    }
    return NULL;
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

FlintJavaObject &Flint::newObject(uint32_t size, FlintConstUtf8 &type, uint8_t dimensions) {
    objectSizeToGc += size;
    if(objectSizeToGc >= OBJECT_SIZE_TO_GC)
        garbageCollection();
    FlintJavaObject *newNode = (FlintJavaObject *)Flint::malloc(sizeof(FlintJavaObject) + size);
    new (newNode)FlintJavaObject(size, type, dimensions);

    Flint::lock();
    newNode->prev = 0;
    newNode->next = objectList;
    if(objectList)
        objectList->prev = newNode;
    objectList = newNode;
    Flint::unlock();

    return *newNode;
}

FlintJavaObject &Flint::newObject(FlintConstUtf8 &type) {
    FlintJavaObject &obj = newObject(sizeof(FlintFieldsData), type, 0);
    memset(obj.data, 0, sizeof(FlintFieldsData));

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)obj.data;
    new (fields)FlintFieldsData(*this, load(type), false);

    return obj;
}

FlintInt8Array &Flint::newBooleanArray(uint32_t length) {
    return *(FlintInt8Array *)&newObject(length, *(FlintConstUtf8 *)primTypeConstUtf8List[0], 1);
}

FlintInt8Array &Flint::newByteArray(uint32_t length) {
    return *(FlintInt8Array *)&newObject(length, *(FlintConstUtf8 *)primTypeConstUtf8List[4], 1);
}

FlintInt16Array &Flint::newCharArray(uint32_t length) {
    return *(FlintInt16Array *)&newObject(length * sizeof(int16_t), *(FlintConstUtf8 *)primTypeConstUtf8List[1], 1);
}

FlintInt16Array &Flint::newShortArray(uint32_t length) {
    return *(FlintInt16Array *)&newObject(length * sizeof(int16_t), *(FlintConstUtf8 *)primTypeConstUtf8List[5], 1);
}

FlintInt32Array &Flint::newIntegerArray(uint32_t length) {
    return *(FlintInt32Array *)&newObject(length * sizeof(int32_t), *(FlintConstUtf8 *)primTypeConstUtf8List[6], 1);
}

FlintFloatArray &Flint::newFloatArray(uint32_t length) {
    return *(FlintFloatArray *)&newObject(length * sizeof(float), *(FlintConstUtf8 *)primTypeConstUtf8List[2], 1);
}

FlintInt64Array &Flint::newLongArray(uint32_t length) {
    return *(FlintInt64Array *)&newObject(length * sizeof(int64_t), *(FlintConstUtf8 *)primTypeConstUtf8List[7], 1);
}

FlintDoubleArray &Flint::newDoubleArray(uint32_t length) {
    return *(FlintDoubleArray *)&newObject(length * sizeof(double), *(FlintConstUtf8 *)primTypeConstUtf8List[3], 1);
}

FlintObjectArray &Flint::newObjectArray(FlintConstUtf8 &type, uint32_t length) {
    return *(FlintObjectArray *)&newObject(length * sizeof(FlintJavaObject *), type, 1);
}

FlintJavaObject &Flint::newMultiArray(FlintConstUtf8 &typeName, uint8_t dimensions, int32_t *counts) {
    if(dimensions > 1) {
        FlintJavaObject &array = newObject(counts[0] * sizeof(FlintJavaObject *), typeName, dimensions);
        for(uint32_t i = 0; i < counts[0]; i++)
            ((FlintJavaObject **)(array.data))[i] = &newMultiArray(typeName, dimensions - 1, &counts[1]);
        return array;
    }
    else {
        uint8_t atype = FlintJavaObject::isPrimType(typeName);
        uint8_t typeSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
        FlintJavaObject &array = newObject(typeSize * counts[0], typeName, 1);
        memset(array.data, 0, array.size);
        return array;
    }
}

FlintJavaClass &Flint::newClass(FlintJavaString &typeName) {
    // TODO - Check the existence of type

    FlintJavaClass &classObj = *(FlintJavaClass *)&newObject(*(FlintConstUtf8 *)&classClassName);

    /* set value for name field */
    classObj.setName(&typeName);

    return classObj;
}

FlintJavaClass &Flint::newClass(const char *typeName, uint16_t length) {
    /* create String object to store typeName */
    FlintJavaString &name = newString(typeName, length);

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

FlintJavaClass &Flint::getConstClass(const char *typeName, uint16_t length) {
    for(FlintConstClass *node = constClassList; node != 0; node = node->next) {
        if(node->flintClass.getName().equals(typeName, length))
            return node->flintClass;
    }
    FlintJavaClass &classObj = newClass(typeName, length);
    FlintConstClass *newNode = (FlintConstClass *)Flint::malloc(sizeof(FlintConstClass));
    new (newNode)FlintConstClass(classObj);

    newNode->next = constClassList;
    constClassList = newNode;

    return classObj;
}

FlintJavaClass &Flint::getConstClass(FlintJavaString &str) {
    for(FlintConstClass *node = constClassList; node != 0; node = node->next) {
        if(node->flintClass.getName().equals(str))
            return node->flintClass;
    }
    FlintJavaClass &classObj = newClass(str);
    FlintConstClass *newNode = (FlintConstClass *)Flint::malloc(sizeof(FlintConstClass));
    new (newNode)FlintConstClass(classObj);

    newNode->next = constClassList;
    constClassList = newNode;

    return classObj;
}

FlintJavaString &Flint::newString(uint16_t length, uint8_t coder) {
    /* create new byte array to store string */
    FlintInt8Array &byteArray = newByteArray(length << (coder ? 1 : 0));

    /* create new string object */
    FlintJavaString &strObj = *(FlintJavaString *)&newObject(*(FlintConstUtf8 *)&stringClassName);

    /* set value for value field */
    strObj.setValue(byteArray);

    /* set value for coder field */
    strObj.setCoder(coder);

    return strObj;
}

FlintJavaString &Flint::newString(const char *text, uint16_t size, bool isUtf8) {
    uint32_t index = 0;
    bool isLatin1 = isUtf8 ? FlintJavaString::isLatin1(text) : true;
    uint32_t strLen = isUtf8 ? FlintJavaString::utf8StrLen(text) : size;

    /* create new byte array to store string */
    uint32_t arrayLen = isLatin1 ? strLen : (strLen << 1);
    FlintInt8Array &byteArray = newByteArray(arrayLen);
    uint8_t *byteArrayData = (uint8_t *)byteArray.getData();
    if(!isUtf8)
        memcpy(byteArrayData, text, strLen);
    else {
        if(isLatin1) {
            while(*text) {
                uint32_t c = FlintJavaString::utf8Decode(text);
                byteArrayData[index] = c;
                text += FlintJavaString::getUtf8DecodeSize(*text);
                index++;
            }
        }
        else while(*text) {
            uint32_t c = FlintJavaString::utf8Decode(text);
            if(c <= 0xFFFFFF)
                ((uint16_t *)byteArrayData)[index] = c;
            else
                throw "Characters are not supported";
            text += FlintJavaString::getUtf8DecodeSize(*text);
            index++;
        }
    }

    /* create new string object */
    FlintJavaString &strObj = *(FlintJavaString *)&newObject(*(FlintConstUtf8 *)&stringClassName);

    /* set value for value field */
    strObj.setValue(byteArray);

    /* set value for coder field */
    strObj.setCoder(isLatin1 ? 0 : 1);

    return strObj;
}

FlintJavaString &Flint::newString(const char *latin1Str[], uint16_t count) {
    uint16_t index = 0;
    uint16_t length = 0;
    for(uint16_t i = 0; i < count; i++)
        length += strlen(latin1Str[i]);

    /* create new byte array to store string */
    FlintInt8Array &byteArray = newByteArray(length);
    uint8_t *byteArrayData = (uint8_t *)byteArray.getData();
    for(uint16_t i = 0; i < count; i++) {
        const char *buff = latin1Str[i];
        while(*buff) {
            uint32_t c = FlintJavaString::utf8Decode(buff);
            byteArrayData[index] = c;
            buff += FlintJavaString::getUtf8DecodeSize(*buff);
            index++;
        }
    }

    /* create new string object */
    FlintJavaString &strObj = *(FlintJavaString *)&newObject(*(FlintConstUtf8 *)&stringClassName);

    /* set value for value field */
    strObj.setValue(byteArray);

    return strObj;
}

FlintJavaString &Flint::getConstString(FlintConstUtf8 &utf8) {
    for(FlintConstString *node = constStringList; node != 0; node = node->next) {
        if(node->flintString.equals(utf8))
            return node->flintString;
    }
    FlintJavaString &strObj = newString(utf8.text, utf8.length, true);
    FlintConstString *newNode = (FlintConstString *)Flint::malloc(sizeof(FlintConstString));
    new (newNode)FlintConstString(strObj);

    newNode->next = constStringList;
    constStringList = newNode;

    return strObj;
}

FlintJavaString &Flint::getConstString(FlintJavaString &str) {
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
    static const FlintConstUtf8 *baseConstUtf8List[] = {
        primTypeConstUtf8List[0],
        primTypeConstUtf8List[1],
        primTypeConstUtf8List[2],
        primTypeConstUtf8List[3],
        primTypeConstUtf8List[4],
        primTypeConstUtf8List[5],
        primTypeConstUtf8List[6],
        primTypeConstUtf8List[7],
        &mathClassName,
        &errorClassName,
        &classClassName,
        &floatClassName,
        &doubleClassName,
        &objectClassName,
        &systemClassName,
        &stringClassName,
        &threadClassName,
        &characterClassName,
        &throwableClassName,
        &exceptionClassName,
        &bigIntegerClassName,
        &printStreamClassName,
        &ioExceptionClassName,
        &flintGraphicsClassName,
        &arrayStoreExceptionClassName,
        &arithmeticExceptionClassName,
        &nullPointerExceptionClassName,
        &unsatisfiedLinkErrorClassName,
        &interruptedExceptionClassName,
        &classNotFoundExceptionClassName,
        &cloneNotSupportedExceptionClassName,
        &negativeArraySizeExceptionClassName,
        &unsupportedOperationExceptionClassName,
        &arrayIndexOutOfBoundsExceptionClassName,
    };

    uint32_t hash;
    ((uint16_t *)&hash)[0] = length;
    ((uint16_t *)&hash)[1] = Flint_CalcCrc((uint8_t *)text, length);

    for(uint32_t i = 0; i < LENGTH(baseConstUtf8List); i++) {
        if(CONST_UTF8_HASH(baseConstUtf8List[i][0]) == hash) {
            if(strncmp(baseConstUtf8List[i]->text, text, length) == 0)
                return *(FlintConstUtf8 *)baseConstUtf8List[i];
        }
    }

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

FlintJavaThrowable &Flint::newThrowable(FlintJavaString *strObj, FlintConstUtf8 &excpType) {
    /* create new exception object */
    FlintJavaThrowable &obj = *(FlintJavaThrowable *)&newObject(excpType);

    /* set detailMessage value */
    if(strObj)
        obj.setDetailMessage(*strObj);

    return obj;
}

FlintJavaThrowable &Flint::newException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&exceptionClassName);
}

FlintJavaThrowable &Flint::newIOException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&exceptionClassName);
}

FlintJavaThrowable &Flint::newErrorException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&errorClassName);
}

FlintJavaThrowable &Flint::newArrayStoreException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&arrayStoreExceptionClassName);
}

FlintJavaThrowable &Flint::newArithmeticException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&arithmeticExceptionClassName);
}

FlintJavaThrowable &Flint::newNullPointerException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&nullPointerExceptionClassName);
}

FlintJavaThrowable &Flint::newInterruptedException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&interruptedExceptionClassName);
}

FlintJavaThrowable &Flint::newClassNotFoundException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&classNotFoundExceptionClassName);
}

FlintJavaThrowable &Flint::newCloneNotSupportedException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&cloneNotSupportedExceptionClassName);
}

FlintJavaThrowable &Flint::newNegativeArraySizeException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&negativeArraySizeExceptionClassName);
}

FlintJavaThrowable &Flint::newArrayIndexOutOfBoundsException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&arrayIndexOutOfBoundsExceptionClassName);
}

FlintJavaThrowable &Flint::newUnsupportedOperationException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&unsupportedOperationExceptionClassName);
}

FlintJavaThrowable &Flint::newUnsatisfiedLinkErrorException(FlintJavaString *strObj) {
    return newThrowable(strObj, *(FlintConstUtf8 *)&unsatisfiedLinkErrorClassName);
}

void Flint::clearProtectObjectNew(FlintJavaObject &obj) {
    bool isPrim = FlintJavaObject::isPrimType(obj.type);
    if((obj.dimensions > 1) || (obj.dimensions == 1 && !isPrim)) {
        uint32_t count = obj.size / 4;
        for(uint32_t i = 0; i < count; i++) {
            FlintJavaObject *tmp = ((FlintJavaObject **)obj.data)[i];
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(*tmp);
        }
    }
    else if(!isPrim) {
        FlintFieldsData &fieldData = *(FlintFieldsData *)obj.data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            FlintJavaObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(*tmp);
        }
    }
    obj.clearProtected();
}

void Flint::garbageCollectionProtectObject(FlintJavaObject &obj) {
    bool isPrim = FlintJavaObject::isPrimType(obj.type);
    obj.setProtected();
    if((obj.dimensions > 1) || (obj.dimensions == 1 && !isPrim)) {
        uint32_t count = obj.size / 4;
        for(uint32_t i = 0; i < count; i++) {
            FlintJavaObject *tmp = ((FlintJavaObject **)obj.data)[i];
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(*tmp);
        }
    }
    else if(!isPrim) {
        FlintFieldsData &fieldData = *(FlintFieldsData *)obj.data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            FlintJavaObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(*tmp);
        }
    }
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
                FlintJavaObject *obj = fieldsData->fieldsObject[i].object;
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(*obj);
            }
        }
    }
    for(FlintExecutionNode *node = executionList; node != 0; node = node->next) {
        if(node->onwerThread && !node->onwerThread->getProtected())
            garbageCollectionProtectObject(*node->onwerThread);
        for(int32_t i = 0; i <= node->peakSp; i++) {
            if(node->getStackType(i) == STACK_TYPE_OBJECT) {
                FlintJavaObject *obj = (FlintJavaObject *)node->stack[i];
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(*obj);
            }
        }
    }
    for(FlintJavaObject *node = objectList; node != 0;) {
        FlintJavaObject *next = node->next;
        uint8_t prot = node->getProtected();
        if(prot == 0) {
            if(node->prev)
                node->prev->next = node->next;
            else
                objectList = node->next;
            if(node->next)
                node->next->prev = node->prev;

            if(node->dimensions == 0)
                node->getFields().~FlintFieldsData();
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
        for(ClassData *node = classDataList; node != 0; node = node->next) {
            if(className == node->getThisClass()) {
                Flint::unlock();
                return *node;
            }
        }
        newNode = (ClassData *)Flint::malloc(sizeof(ClassData));
        newNode->staticFieldsData = 0;
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

bool Flint::isInstanceof(FlintJavaObject *obj, const char *typeName, uint16_t length) {
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
    newExecution(NULL, stackSize).run(load(mainClass).getMainMethodInfo());
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

void Flint::freeObject(FlintJavaObject &obj) {
    Flint::lock();
    if(obj.prev)
        obj.prev->next = obj.next;
    else
        objectList = obj.next;
    if(obj.next)
        obj.next->prev = obj.prev;

    if(obj.dimensions == 0)
        obj.getFields().~FlintFieldsData();
    Flint::free(&obj);
    Flint::unlock();
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
    for(FlintJavaObject *node = objectList; node != 0;) {
        FlintJavaObject *next = node->next;
        if(node->dimensions == 0)
            node->getFields().~FlintFieldsData();
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

void Flint::reset(void) {
    FlintAPI::System::reset(*this);
}
