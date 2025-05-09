
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_const_name_base_hash_table.h"

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
    }
    objectCount++;
    return ret;
}

void *Flint::realloc(void *p, uint32_t size) {
    void *ret = FlintAPI::System::realloc(p, size);
    if(ret == 0) {
        flintInstance.garbageCollection();
        ret = FlintAPI::System::realloc(p, size);
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

Flint::Flint(void): classDataTree(), constClassTree(), constStringTree(), constUtf8Tree() {
    dbg = 0;
    executionList = 0;
    objectList = 0;
    classArray0 = 0;
    objectSizeToGc = 0;
}

FlintDebugger *Flint::getDebugger(void) const {
    return dbg;
}

void Flint::setDebugger(FlintDebugger *dbg) {
    this->dbg = dbg;
}

void Flint::print(const char *text) {
    uint32_t len = strlen(text);
    print(text, len, 0);
}

void Flint::print(const FlintConstUtf8 &utf8) {
    print(utf8.text, utf8.length, 0);
}

void Flint::print(FlintJavaString *str) {
    print(str->getText(), str->getLength(), str->getCoder());
}

void Flint::print(const char *text, uint32_t length, uint8_t coder) {
    if(dbg)
        dbg->print(text, length, coder);
    else
        FlintAPI::System::print(text, length, coder);
}

void Flint::println(const char *text) {
    uint32_t len = strlen(text);
    print(text, len, 0);
    print("\n", 1, 0);
}

void Flint::println(const FlintConstUtf8 &utf8) {
    print(utf8.text, utf8.length, 0);
    print("\n", 1, 0);
}

void Flint::println(FlintJavaString *str) {
    print(str->getText(), str->getLength(), str->getCoder());
    print("\n", 1, 0);
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

FlintError Flint::newObject(uint32_t size, const FlintConstUtf8 &type, uint8_t dimensions, FlintJavaObject *&obj) {
    objectSizeToGc += size;
    if(objectSizeToGc >= OBJECT_SIZE_TO_GC)
        garbageCollection();
    FlintJavaObject *newNode = (FlintJavaObject *)Flint::malloc(sizeof(FlintJavaObject) + size);
    if(newNode == NULL)
        return ERR_OUT_OF_MEMORY;
    new (newNode)FlintJavaObject(size, type, dimensions);

    Flint::lock();
    newNode->prev = 0;
    newNode->next = objectList;
    if(objectList)
        objectList->prev = newNode;
    objectList = newNode;
    Flint::unlock();

    obj = newNode;

    return ERR_OK;
}

FlintError Flint::newObject(const FlintConstUtf8 &type, FlintJavaObject *&obj) {
    RETURN_IF_ERR(newObject(sizeof(FlintFieldsData), type, 0, obj));
    memset(obj->data, 0, sizeof(FlintFieldsData));

    FlintClassLoader *loader;
    FlintError err = load(type, loader);
    if(err != ERR_OK) {
        freeObject(*obj);
        return err;
    }

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)obj->data;
    new (fields)FlintFieldsData();
    FlintConstUtf8 *classError;
    err = fields->loadNonStatic(*this, *loader, classError);
    if(err != ERR_OK) {
        obj = (FlintJavaObject *)classError;
        return err;
    }

    return ERR_OK;
}

FlintError Flint::newBooleanArray(uint32_t length, FlintInt8Array *&array) {
    return newObject(length, booleanPrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newByteArray(uint32_t length, FlintInt8Array *&array) {
    return newObject(length, bytePrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newCharArray(uint32_t length, FlintInt16Array *&array) {
    return newObject(length * sizeof(int16_t), charPrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newShortArray(uint32_t length, FlintInt16Array *&array) {
    return newObject(length * sizeof(int16_t), shortPrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newIntegerArray(uint32_t length, FlintInt32Array *&array) {
    return newObject(length * sizeof(int32_t), integerPrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newFloatArray(uint32_t length, FlintFloatArray *&array) {
    return newObject(length * sizeof(float), floatPrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newLongArray(uint32_t length, FlintInt64Array *&array) {
    return newObject(length * sizeof(int64_t), longPrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newDoubleArray(uint32_t length, FlintDoubleArray *&array) {
    return newObject(length * sizeof(double), doublePrimTypeName, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newObjectArray(const FlintConstUtf8 &type, uint32_t length, FlintObjectArray *&array) {
    return newObject(length * sizeof(FlintJavaObject *), type, 1, (FlintJavaObject *&)array);
}

FlintError Flint::newMultiArray(const FlintConstUtf8 &typeName, int32_t *counts, uint8_t startDims, uint8_t endDims, FlintJavaObject *&array) {
    if(startDims > 1) {
        RETURN_IF_ERR(newObject(counts[0] * sizeof(FlintJavaObject *), typeName, startDims, array));
        if(startDims > endDims) {
            for(uint32_t i = 0; i < counts[0]; i++) {
                FlintError err = newMultiArray(typeName, &counts[1], startDims - 1, endDims, ((FlintJavaObject **)(array->data))[i]);
                if(err != ERR_OK) {
                    for(uint32_t j = 0; j < i; j++)
                        freeObject(*((FlintJavaObject **)(array->data))[j]);
                    return err;
                }
            }
        }
        else
            memset(array->data, 0, array->size);
    }
    else {
        uint8_t atype = FlintJavaObject::isPrimType(typeName);
        uint8_t typeSize = atype ? FlintJavaObject::getPrimitiveTypeSize(atype) : sizeof(FlintJavaObject *);
        RETURN_IF_ERR(newObject(typeSize * counts[0], typeName, 1, array));
        memset(array->data, 0, array->size);
    }
    return ERR_OK;
}

FlintError Flint::newClass(FlintJavaString &typeName, FlintJavaClass *&cls) {
    // TODO - Check the existence of type

    RETURN_IF_ERR(newObject(classClassName, (FlintJavaObject *&)cls));

    /* set value for name field */
    cls->setName(&typeName);

    return ERR_OK;
}

FlintError Flint::newClass(const char *typeName, uint16_t length, FlintJavaClass *&cls) {
    /* create String object to store typeName */
    FlintJavaString *name;
    FlintError err = newString(typeName, length, false, name);
    if(err != ERR_OK) {
        /* return class name in case class not found or error */
        cls = (FlintJavaClass *)name;
        return err;
    }

    /* replace '/' to '.' */
    char *text = (char *)name->getText();
    for(uint32_t i = 0; i < length; i++) {
        if(text[i] == '/')
            text[i] = '.';
    }
    err = newClass(*name, cls);
    if(err != ERR_OK)
        freeObject(*name);
    return err;
}

FlintError Flint::getConstClass(const char *typeName, uint16_t length, FlintJavaClass *&cls) {
    if(*typeName == 'L') {
        length -= (typeName[length - 1] == ';') ? 2 : 1;
        typeName++;
    }

    Flint::lock();
    cls = constClassTree.find(typeName, length);
    if(!cls) {
        FlintError err = newClass(typeName, length, cls);
        if(err != ERR_OK) {
            Flint::unlock();
            return err;
        }
        constClassTree.add(*cls);
    }
    Flint::unlock();

    return ERR_OK;
}

FlintError Flint::getConstClass(FlintJavaString &str, FlintJavaClass *&cls) {
    Flint::lock();
    cls = constClassTree.find(str);
    if(!cls) {
        FlintError err = newClass(str, cls);
        if(err != ERR_OK) {
            Flint::unlock();
            return err;
        }
        constClassTree.add(*cls);
    }
    Flint::unlock();

    return ERR_OK;
}

FlintError Flint::newString(uint16_t length, uint8_t coder, FlintJavaString *&str) {
    /* create new byte array to store string */
    FlintInt8Array *byteArray;
    RETURN_IF_ERR(newByteArray(length << (coder ? 1 : 0), byteArray));

    /* create new string object */
    FlintError err = newObject(stringClassName, (FlintJavaObject *&)str);
    if(err != ERR_OK) {
        freeObject(*byteArray);
        return err;
    }

    /* set value for value field */
    str->setValue(*byteArray);
    /* set value for coder field */
    str->setCoder(coder);

    return ERR_OK;
}

FlintError Flint::newString(const char *text, FlintJavaString *&str) {
    uint32_t len = strlen(text);
    return newString(text, len, false, str);
}

FlintError Flint::newString(const char *text, uint16_t size, bool isUtf8, FlintJavaString *&str) {
    uint32_t index = 0;
    bool isLatin1 = isUtf8 ? FlintJavaString::isLatin1(text) : true;
    uint32_t strLen = isUtf8 ? FlintJavaString::utf8StrLen(text) : size;

    /* create new byte array to store string */
    uint32_t arrayLen = isLatin1 ? strLen : (strLen << 1);
    FlintInt8Array *byteArray;
    RETURN_IF_ERR(newByteArray(arrayLen, byteArray));
    uint8_t *byteArrayData = (uint8_t *)byteArray->getData();
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
            ((uint16_t *)byteArrayData)[index] = c;
            text += FlintJavaString::getUtf8DecodeSize(*text);
            index++;
        }
    }

    /* create new string object */
    FlintError err = newObject(stringClassName, (FlintJavaObject *&)str);
    if(err != ERR_OK) {
        freeObject(*byteArray);
        return err;
    }

    /* set value for value field */
    str->setValue(*byteArray);
    /* set value for coder field */
    str->setCoder(isLatin1 ? 0 : 1);

    return ERR_OK;
}

FlintError Flint::getConstString(const FlintConstUtf8 &utf8, FlintJavaString *&str) {
    Flint::lock();
    str = constStringTree.find(utf8);
    if(!str) {
        FlintError err = newString(utf8.text, utf8.length, true, str);
        if(err != ERR_OK) {
            Flint::unlock();
            return err;
        }
        constStringTree.add(*str);
    }
    Flint::unlock();
    return ERR_OK;
}

FlintError Flint::getConstString(FlintJavaString &str, FlintJavaString *&strRet) {
    Flint::lock();
    strRet = constStringTree.find(str);
    if(!strRet)
        strRet = &constStringTree.add(str);
    Flint::unlock();
    return ERR_OK;
}

static FlintConstUtf8 *getConstUtf8InBaseConstName(const char *text, uint32_t hash, bool isTypeName) {
    uint32_t hashIndex = Flint_HashIndex(hash, LENGTH(baseConstUtf8HashTable));
    if(baseConstUtf8HashTable[hashIndex]) {
        int32_t left = 0;
        int32_t right = baseConstUtf8HashTable[hashIndex]->count - 1;
        const FlintConstUtf8 * const *constUtf8List = baseConstUtf8HashTable[hashIndex]->values;
        while(left <= right) {
            int32_t mid = left + (right - left) / 2;
            int32_t compareResult = FlintConstUtf8BinaryTree::compareConstUtf8(text, hash, (FlintConstUtf8 &)*constUtf8List[mid], isTypeName);
            if(compareResult == 0)
                return (FlintConstUtf8 *)constUtf8List[mid];
            else if(compareResult > 0)
                left = mid + 1;
            else
                right = mid - 1;
        }
    }
    return NULL;
}

FlintConstUtf8 &Flint::getConstUtf8(const char *text, uint16_t length) {
    uint32_t hash = Flint_CalcHash(text, length, false);
    FlintConstUtf8 *ret = getConstUtf8InBaseConstName(text, hash, false);
    if(ret)
        return *ret;

    Flint::lock();
    ret = constUtf8Tree.find(text, hash, false);
    if(!ret)
        ret = &constUtf8Tree.add(text, hash, false);
    Flint::unlock();

    return *ret;
}

FlintConstUtf8 &Flint::getTypeNameConstUtf8(const char *typeName, uint16_t length) {
    uint32_t hash = Flint_CalcHash(typeName, length, true);
    FlintConstUtf8 *ret = getConstUtf8InBaseConstName(typeName, hash, true);
    if(ret)
        return *ret;

    Flint::lock();
    ret = constUtf8Tree.find(typeName, hash, true);
    if(!ret)
        ret = &constUtf8Tree.add(typeName, hash, true);
    Flint::unlock();

    return *ret;
}

FlintError Flint::getClassArray0(FlintObjectArray *&obj) {
    if(classArray0) {
        obj = (FlintObjectArray *)classArray0;
        return ERR_OK;
    }
    Flint::lock();
    if(classArray0 == NULL) {
        RETURN_IF_ERR(newObjectArray(classClassName, 0, obj));
        classArray0 = obj;
    }
    else {
        Flint::unlock();
        obj = (FlintObjectArray *)classArray0;
    }
    return ERR_OK;
}

FlintError Flint::newThrowable(FlintJavaString *str, const FlintConstUtf8 &excpType, FlintJavaThrowable *&excp) {
    /* create new exception object */
    RETURN_IF_ERR(newObject(excpType, (FlintJavaObject *&)excp));

    /* set detailMessage value */
    if(str)
        excp->setDetailMessage(*str);

    return ERR_OK;
}

FlintError Flint::newBoolean(bool value, FlintJavaBoolean *&obj) {
    RETURN_IF_ERR(newObject(booleanClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newByte(int8_t value, FlintJavaByte *&obj) {
    RETURN_IF_ERR(newObject(byteClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newCharacter(uint16_t value, FlintJavaCharacter *&obj) {
    RETURN_IF_ERR(newObject(characterClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newShort(int16_t value, FlintJavaShort *&obj) {
    RETURN_IF_ERR(newObject(shortClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newInteger(int32_t value, FlintJavaInteger *&obj) {
    RETURN_IF_ERR(newObject(integerClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newFloat(float value, FlintJavaFloat *&obj) {
    RETURN_IF_ERR(newObject(floatClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newLong(int64_t value, FlintJavaLong *&obj) {
    RETURN_IF_ERR(newObject(longClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
}

FlintError Flint::newDouble(double value, FlintJavaDouble *&obj) {
    RETURN_IF_ERR(newObject(doubleClassName, (FlintJavaObject *&)obj));
    obj->setValue(value);
    return ERR_OK;
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

static bool checkAddressIsValid(uint32_t address) {
    if(address & 0x03)
        return false;
    if(!FlintAPI::System::isInHeapRegion((void *)address))
        return false;
    return true;
}

bool Flint::isObject(uint32_t address) const {
    if(!address)
        return false;
    if(!checkAddressIsValid(address))
        return false;
    FlintJavaObject *obj = (FlintJavaObject *)address;
    if(obj->prev == 0)
        return address == (uint32_t)objectList;
    if(!checkAddressIsValid((uint32_t)obj->prev))
        return false;
    if((uint32_t)obj->prev->next != address)
        return false;
    if(obj->next) {
        if(!checkAddressIsValid((uint32_t)obj->next))
            return false;
        return (uint32_t)obj->next->prev == address;
    }
    return true;
}

void Flint::clearProtectObjectNew(FlintJavaObject &obj) {
    bool isPrim = FlintJavaObject::isPrimType(obj.type);
    obj.clearProtected();
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
}

void Flint::garbageCollection(void) {
    Flint::lock();
    objectSizeToGc = 0;
    constClassTree.forEach([](FlintJavaClass &item) {
        if(!item.getProtected())
            garbageCollectionProtectObject(item);
    });
    constStringTree.forEach([](FlintJavaString &item) {
        if(!item.getProtected())
            garbageCollectionProtectObject(item);
    });
    classDataTree.forEach([](FlintClassData &item) {
        FlintFieldsData *fieldsData = item.staticFieldsData;
        if(fieldsData && fieldsData->fieldsObjCount) {
            for(uint32_t i = 0; i < fieldsData->fieldsObjCount; i++) {
                FlintJavaObject *obj = fieldsData->fieldsObject[i].object;
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(*obj);
            }
        }
    });
    for(FlintExecutionNode *node = executionList; node != 0; node = node->next) {
        if(node->onwerThread && !node->onwerThread->getProtected())
            garbageCollectionProtectObject(*node->onwerThread);
        int32_t startSp = node->startSp;
        int32_t endSp = FLINT_MAX(node->sp, node->peakSp);
        while(startSp >= 3) {
            for(int32_t i = startSp; i <= endSp; i++) {
                if(isObject(node->stack[i])) {
                    FlintJavaObject *obj = (FlintJavaObject *)node->stack[i];
                    if(obj && !obj->getProtected())
                        garbageCollectionProtectObject(*obj);
                }
            }
            endSp = startSp - 4;
            startSp = node->stack[startSp];
        }
    }
    if(classArray0 && !((FlintObjectArray *)classArray0)->getProtected())
        garbageCollectionProtectObject(*(FlintJavaObject *)classArray0);
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

FlintError Flint::createFlintClassData(Flint *flint, const char *className, uint16_t length, FlintClassData *&classData) {
    classData = (FlintClassData *)Flint::malloc(sizeof(FlintClassData));
    if(classData == NULL)
        return ERR_OUT_OF_MEMORY;
    classData->staticFieldsData = 0;
    new (classData)FlintClassData(*flint);
    return classData->load(className, length);
}

FlintError Flint::load(const char *className, uint16_t length, FlintClassLoader *&loader) {
    Flint::lock();
    loader = (FlintClassLoader *)classDataTree.find(className, length);
    if(!loader) {
        FlintError err = createFlintClassData(this, className, length, (FlintClassData *&)loader);
        if(err != ERR_OK) {
            Flint::unlock();
            return err;
        }
        classDataTree.add(*(FlintClassData *)loader);
    }
    Flint::unlock();
    return ERR_OK;
}

FlintError Flint::load(const char *className, FlintClassLoader *&loader) {
    return load(className, strlen(className), loader);
}

FlintError Flint::load(const FlintConstUtf8 &className, FlintClassLoader *&loader) {
    Flint::lock();
    loader = (FlintClassLoader *)classDataTree.find(className);
    if(!loader) {
        FlintError err = createFlintClassData(this, className.text, className.length, (FlintClassData *&)loader);
        if(err != ERR_OK) {
            Flint::unlock();
            return err;
        }
        classDataTree.add(*(FlintClassData *)loader);
    }
    Flint::unlock();
    return ERR_OK;
}

FlintError Flint::initStaticField(FlintClassData &classData) {
    FlintFieldsData *fieldsData = (FlintFieldsData *)Flint::malloc(sizeof(FlintFieldsData));
    if(fieldsData == NULL)
        return ERR_OUT_OF_MEMORY;
    new (fieldsData)FlintFieldsData();
    fieldsData->loadStatic(classData);
    classData.staticFieldsData = fieldsData;
    return ERR_OK;
}

FlintError Flint::findMethod(FlintConstMethod &constMethod, FlintMethodInfo *&methodInfo) {
    FlintClassLoader *loader;
    RETURN_IF_ERR(load(constMethod.className, loader));
    while(loader) {
        FlintError err = loader->getMethodInfo(constMethod.nameAndType, methodInfo);
        if(err == ERR_OK)
            return ERR_OK;
        else if(err != ERR_METHOD_NOT_FOUND) {
            methodInfo = (FlintMethodInfo *)loader->thisClass;
            return err;
        }
        FlintConstUtf8 *superClass = loader->superClass;
        if(!superClass)
            return ERR_METHOD_NOT_FOUND;
        err = load(*superClass, loader);
        if(err != ERR_OK) {
            methodInfo = (FlintMethodInfo *)superClass;
            return err;
        }
    }
    return ERR_METHOD_NOT_FOUND;
}

FlintError Flint::findMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType, FlintMethodInfo *&methodInfo) {
    FlintClassLoader *loader;
    RETURN_IF_ERR(load(className, loader));
    while(loader) {
        FlintError err = loader->getMethodInfo(nameAndType, methodInfo);
        if(err == ERR_OK)
            return ERR_OK;
        else if(err != ERR_METHOD_NOT_FOUND) {
            methodInfo = (FlintMethodInfo *)loader->thisClass;
            return err;
        }
        FlintConstUtf8 *superClass = loader->superClass;
        if(!superClass)
            return ERR_METHOD_NOT_FOUND;
        err = load(*superClass, loader);
        if(err != ERR_OK) {
            methodInfo = (FlintMethodInfo *)superClass;
            return err;
        }
    }
    return ERR_METHOD_NOT_FOUND;
}

static bool compareClassName(const FlintConstUtf8 &className1, const char *className2, uint32_t hash) {
    if(CONST_UTF8_HASH(className1) != hash)
        return false;
    const char *txt1 = className1.text;
    uint16_t length = className1.length;
    for(uint16_t i = 0; i < length; i++) {
        if(txt1[i] == className2[i])
            continue;
        else if((txt1[i] == '.' && className2[i] == '/') || (txt1[i] == '/' && className2[i] == '.'))
            continue;
        return false;
    }
    return true;
}

static uint32_t getDimensions(const char *typeName) {
    const char *text = typeName;
    while(*text == '[')
        text++;
    return (uint32_t)(text - typeName);
}

FlintError Flint::isInstanceof(FlintJavaObject *obj, const char *typeName, uint16_t length, FlintConstUtf8 **classError) {
    uint32_t dimensions = getDimensions(typeName);
    typeName += dimensions;
    length -= dimensions;
    if(*typeName == 'L') {
        length -= (typeName[length - 1] == ';') ? 2 : 1;
        typeName++;
    }
    uint32_t typeNameHash = Flint_CalcHash(typeName, length, true);
    if((obj->dimensions >= dimensions) && compareClassName(objectClassName, typeName, typeNameHash))
        return ERR_OK;
    if(dimensions != obj->dimensions)
        return ERR_IS_INSTANCE_FALSE;
    FlintConstUtf8 *objType = &obj->type;
    if(FlintJavaObject::isPrimType(*objType) || ((length == 1) && FlintJavaObject::convertToAType(typeName[0])))
        return ((length == objType->length) && (typeName[0] == objType->text[0])) ? ERR_OK : ERR_IS_INSTANCE_FALSE;
    while(1) {
        if(compareClassName(*objType, typeName, typeNameHash))
            return ERR_OK;
        FlintClassLoader *loader;
        FlintError err = load(*objType, loader);
        if(err != ERR_OK) {
            if(classError)
                *classError = objType;
            return err;
        }
        uint16_t interfacesCount = loader->getInterfacesCount();
        for(uint32_t i = 0; i < interfacesCount; i++) {
            if(compareClassName(loader->getInterface(i), typeName, typeNameHash))
                return ERR_OK;
        }
        objType = loader->superClass;
        if(objType == NULL)
            return ERR_IS_INSTANCE_FALSE;
    }
}

FlintError Flint::isInstanceof(FlintJavaObject *obj, const FlintConstUtf8 &typeName, FlintConstUtf8 **classError) {
    if(typeName.text[0] == '[' || typeName.text[typeName.length - 1] == ';')
        return isInstanceof(obj, typeName.text, typeName.length, classError);
    return isInstanceof(obj->type, obj->dimensions, typeName, 0, classError);
}

FlintError Flint::isInstanceof(const FlintConstUtf8 &typeName1, uint32_t dimensions1, const FlintConstUtf8 &typeName2, uint32_t dimensions2, FlintConstUtf8 **classError) {
    if((dimensions1 >= dimensions2) && (typeName2 == objectClassName))
        return ERR_OK;
    if(dimensions1 != dimensions2)
        return ERR_IS_INSTANCE_FALSE;
    if(FlintJavaObject::isPrimType(typeName1) || FlintJavaObject::isPrimType(typeName2))
        return (typeName1.text[0] == typeName2.text[0]) ? ERR_OK : ERR_IS_INSTANCE_FALSE;
    const FlintConstUtf8 *objType = &typeName1;
    while(1) {
        if(*objType == typeName2)
            return ERR_OK;
        FlintClassLoader *loader;
        FlintError err = load(*objType, loader);
        if(err != ERR_OK) {
            if(classError)
                *classError = (FlintConstUtf8 *)objType;
            return err;
        }
        uint16_t interfacesCount = loader->getInterfacesCount();
        for(uint32_t i = 0; i < interfacesCount; i++) {
            if(loader->getInterface(i) == typeName2)
                return ERR_OK;
        }
        objType = loader->superClass;
        if(objType == NULL)
            return ERR_IS_INSTANCE_FALSE;
    }
}

FlintError Flint::runToMain(const char *mainClass) {
    FlintMethodInfo *mainMethodInfo;
    FlintClassLoader *loader;
    RETURN_IF_ERR(load(mainClass, loader));
    RETURN_IF_ERR(loader->getMainMethodInfo(mainMethodInfo));
    return newExecution().run(mainMethodInfo) ? ERR_OK : ERR_OUT_OF_MEMORY;
}

FlintError Flint::runToMain(const char *mainClass, uint32_t stackSize) {
    FlintMethodInfo *mainMethodInfo;
    FlintClassLoader *loader;
    RETURN_IF_ERR(load(mainClass, loader));
    RETURN_IF_ERR(loader->getMainMethodInfo(mainMethodInfo));
    return newExecution(NULL, stackSize).run(mainMethodInfo) ? ERR_OK : ERR_OUT_OF_MEMORY;
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
    classDataTree.forEach([](FlintClassData &item) {
        item.staticInitOwnId = 0;
        item.clearStaticFields();
    });
}

void Flint::freeAllObject(void) {
    Flint::lock();
    constClassTree.clear();
    constStringTree.clear();
    for(FlintJavaObject *node = objectList; node != 0;) {
        FlintJavaObject *next = node->next;
        if(node->dimensions == 0)
            node->getFields().~FlintFieldsData();
        Flint::free(node);
        node = next;
    }
    objectList = 0;
    classArray0 = 0;
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
    classDataTree.clear();
    Flint::unlock();
}

void Flint::freeAllConstUtf8(void) {
    Flint::lock();
    constUtf8Tree.clear();
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
