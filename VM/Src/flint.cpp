
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

FlintJavaObject &Flint::newObject(uint32_t size, const FlintConstUtf8 &type, uint8_t dimensions) {
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

FlintJavaObject &Flint::newObject(const FlintConstUtf8 &type) {
    FlintJavaObject &obj = newObject(sizeof(FlintFieldsData), type, 0);
    memset(obj.data, 0, sizeof(FlintFieldsData));

    /* init field data */
    FlintFieldsData *fields = (FlintFieldsData *)obj.data;
    new (fields)FlintFieldsData(*this, load(type), false);

    return obj;
}

FlintInt8Array &Flint::newBooleanArray(uint32_t length) {
    return *(FlintInt8Array *)&newObject(length, booleanPrimTypeName, 1);
}

FlintInt8Array &Flint::newByteArray(uint32_t length) {
    return *(FlintInt8Array *)&newObject(length, bytePrimTypeName, 1);
}

FlintInt16Array &Flint::newCharArray(uint32_t length) {
    return *(FlintInt16Array *)&newObject(length * sizeof(int16_t), charPrimTypeName, 1);
}

FlintInt16Array &Flint::newShortArray(uint32_t length) {
    return *(FlintInt16Array *)&newObject(length * sizeof(int16_t), shortPrimTypeName, 1);
}

FlintInt32Array &Flint::newIntegerArray(uint32_t length) {
    return *(FlintInt32Array *)&newObject(length * sizeof(int32_t), integerPrimTypeName, 1);
}

FlintFloatArray &Flint::newFloatArray(uint32_t length) {
    return *(FlintFloatArray *)&newObject(length * sizeof(float), floatPrimTypeName, 1);
}

FlintInt64Array &Flint::newLongArray(uint32_t length) {
    return *(FlintInt64Array *)&newObject(length * sizeof(int64_t), longPrimTypeName, 1);
}

FlintDoubleArray &Flint::newDoubleArray(uint32_t length) {
    return *(FlintDoubleArray *)&newObject(length * sizeof(double), doublePrimTypeName, 1);
}

FlintObjectArray &Flint::newObjectArray(const FlintConstUtf8 &type, uint32_t length) {
    return *(FlintObjectArray *)&newObject(length * sizeof(FlintJavaObject *), type, 1);
}

FlintJavaObject &Flint::newMultiArray(const FlintConstUtf8 &typeName, int32_t *counts, uint8_t startDims, uint8_t endDims) {
    if(startDims > 1) {
        FlintJavaObject &array = newObject(counts[0] * sizeof(FlintJavaObject *), typeName, startDims);
        if(startDims > endDims) {
            for(uint32_t i = 0; i < counts[0]; i++)
                ((FlintJavaObject **)(array.data))[i] = &newMultiArray(typeName, &counts[1], startDims - 1, endDims);
        }
        else
            memset(array.data, 0, array.size);
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

    FlintJavaClass &classObj = *(FlintJavaClass *)&newObject(classClassName);
    /* set value for name field */
    classObj.setName(&typeName);

    return classObj;
}

FlintJavaClass &Flint::newClass(const char *typeName, uint16_t length) {
    /* create String object to store typeName */
    FlintJavaString &name = newString(typeName, length, false);

    /* replace '/' to '.' */
    char *text = (char *)name.getText();
    for(uint32_t i = 0; i < length; i++) {
        if(text[i] == '/')
            text[i] = '.';
    }
    return newClass(name);
}

FlintJavaClass &Flint::getConstClass(const char *typeName, uint16_t length) {
    if(*typeName == 'L') {
        length -= (typeName[length - 1] == ';') ? 2 : 1;
        typeName++;
    }

    Flint::lock();
    FlintJavaClass *ret = constClassTree.find(typeName, length);
    if(!ret)
        ret = &constClassTree.add(newClass(typeName, length));
    Flint::unlock();

    return *ret;
}

FlintJavaClass &Flint::getConstClass(FlintJavaString &str) {
    Flint::lock();
    FlintJavaClass *ret = constClassTree.find(str);
    if(!ret)
        ret = &constClassTree.add(newClass(str));
    Flint::unlock();

    return *ret;
}

FlintJavaString &Flint::newString(uint16_t length, uint8_t coder) {
    /* create new byte array to store string */
    FlintInt8Array &byteArray = newByteArray(length << (coder ? 1 : 0));

    /* create new string object */
    FlintJavaString &str = *(FlintJavaString *)&newObject(stringClassName);
    /* set value for value field */
    str.setValue(byteArray);
    /* set value for coder field */
    str.setCoder(coder);

    return str;
}

FlintJavaString &Flint::newString(const char *text) {
    uint32_t len = strlen(text);
    return newString(text, len, false);
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
            ((uint16_t *)byteArrayData)[index] = c;
            text += FlintJavaString::getUtf8DecodeSize(*text);
            index++;
        }
    }

    /* create new string object */
    FlintJavaString &str = *(FlintJavaString *)&newObject(stringClassName);
    /* set value for value field */
    str.setValue(byteArray);
    /* set value for coder field */
    str.setCoder(isLatin1 ? 0 : 1);

    return str;
}

FlintJavaString &Flint::getConstString(const FlintConstUtf8 &utf8) {
    Flint::lock();
    FlintJavaString *ret = constStringTree.find(utf8);
    if(!ret)
        ret = &constStringTree.add(newString(utf8.text, utf8.length, true));
    Flint::unlock();
    return *ret;
}

FlintJavaString &Flint::getConstString(FlintJavaString &str) {
    Flint::lock();
    FlintJavaString *ret = constStringTree.find(str);
    if(!ret)
        ret = &constStringTree.add(str);
    Flint::unlock();
    return *ret;
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

FlintObjectArray &Flint::getClassArray0(void) {
    if(classArray0)
        return *(FlintObjectArray *)classArray0;
    Flint::lock();
    if(classArray0 == NULL)
        classArray0 = &newObjectArray(classClassName, 0);
    Flint::unlock();
    return *(FlintObjectArray *)classArray0;
}

FlintJavaThrowable &Flint::newThrowable(FlintJavaString *str, const FlintConstUtf8 &excpType) {
    /* create new exception object */
    FlintJavaThrowable &obj = *(FlintJavaThrowable *)&newObject(excpType);

    /* set detailMessage value */
    if(str)
        obj.setDetailMessage(*str);

    return obj;
}

FlintJavaBoolean &Flint::newBoolean(bool value) {
    FlintJavaBoolean &obj = *(FlintJavaBoolean *)&newObject(booleanClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaByte &Flint::newByte(int8_t value) {
    FlintJavaByte &obj = *(FlintJavaByte *)&newObject(byteClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaCharacter &Flint::newCharacter(uint16_t value) {
    FlintJavaCharacter &obj = *(FlintJavaCharacter *)&newObject(characterClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaShort &Flint::newShort(int16_t value) {
    FlintJavaShort &obj = *(FlintJavaShort *)&newObject(shortClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaInteger &Flint::newInteger(int32_t value) {
    FlintJavaInteger &obj = *(FlintJavaInteger *)&newObject(integerClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaFloat &Flint::newFloat(float value) {
    FlintJavaFloat &obj = *(FlintJavaFloat *)&newObject(floatClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaLong &Flint::newLong(int64_t value) {
    FlintJavaLong &obj = *(FlintJavaLong *)&newObject(longClassName);
    obj.setValue(value);
    return obj;
}

FlintJavaDouble &Flint::newDouble(double value) {
    FlintJavaDouble &obj = *(FlintJavaDouble *)&newObject(doubleClassName);
    obj.setValue(value);
    return obj;
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

FlintClassLoader &Flint::load(const char *className, uint16_t length) {
    Flint::lock();
    try {
        FlintClassData *classData = classDataTree.find(className, length);
        if(!classData)
            classData = &classDataTree.add(this, className, length);
        Flint::unlock();
        return *classData;
    }
    catch(const char *msg) {
        Flint::unlock();
        throw (FlintLoadFileError *)className;
    }
}

FlintClassLoader &Flint::load(const char *className) {
    return load(className, strlen(className));
}

FlintClassLoader &Flint::load(const FlintConstUtf8 &className) {
    Flint::lock();
    try {
        FlintClassData *classData = classDataTree.find(className);
        if(!classData)
            classData = &classDataTree.add(this, className);
        Flint::unlock();
        return *classData;
    }
    catch(const char *msg) {
        Flint::unlock();
        throw (FlintLoadFileError *)className.text;
    }
}

void Flint::initStaticField(FlintClassData &classData) {
    FlintFieldsData *fieldsData = (FlintFieldsData *)Flint::malloc(sizeof(FlintFieldsData));
    new (fieldsData)FlintFieldsData(*this, classData, true);
    classData.staticFieldsData = fieldsData;
}

FlintError Flint::findMethod(FlintConstMethod &constMethod, FlintMethodInfo *&methodInfo) {
    FlintClassLoader *loader = &load(constMethod.className);
    while(loader) {
        methodInfo = loader->getMethodInfo(constMethod.nameAndType);
        if(methodInfo)
            return ERR_OK;
        FlintConstUtf8 *superClass = loader->superClass;
        loader = superClass ? &load(*superClass) : NULL;
    }
    return ERR_METHOD_NOT_FOUND;
}

FlintError Flint::findMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType, FlintMethodInfo *&methodInfo) {
    FlintClassLoader *loader = &load(className);
    while(loader) {
        methodInfo = loader->getMethodInfo(nameAndType);
        if(methodInfo)
            return ERR_OK;
        FlintConstUtf8 *superClass = loader->superClass;
        loader = superClass ? &load(*superClass) : NULL;
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

bool Flint::isInstanceof(FlintJavaObject *obj, const char *typeName, uint16_t length) {
    uint32_t dimensions = getDimensions(typeName);
    typeName += dimensions;
    length -= dimensions;
    if(*typeName == 'L') {
        length -= (typeName[length - 1] == ';') ? 2 : 1;
        typeName++;
    }
    uint32_t typeNameHash = Flint_CalcHash(typeName, length, true);
    if((obj->dimensions >= dimensions) && compareClassName(objectClassName, typeName, typeNameHash))
        return true;
    if(dimensions != obj->dimensions)
        return false;
    FlintConstUtf8 *objType = &obj->type;
    if(FlintJavaObject::isPrimType(*objType) || ((length == 1) && FlintJavaObject::convertToAType(typeName[0])))
        return (length == objType->length) && (typeName[0] == objType->text[0]);
    while(1) {
        if(compareClassName(*objType, typeName, typeNameHash))
            return true;
        FlintClassLoader &loader = load(*objType);
        uint16_t interfacesCount = loader.getInterfacesCount();
        for(uint32_t i = 0; i < interfacesCount; i++) {
            if(compareClassName(loader.getInterface(i), typeName, typeNameHash))
                return true;
        }
        objType = loader.superClass;
        if(objType == NULL)
            return false;
    }
}

bool Flint::isInstanceof(FlintJavaObject *obj, const FlintConstUtf8 &typeName) {
    if(typeName.text[0] == '[' || typeName.text[typeName.length - 1] == ';')
        return isInstanceof(obj, typeName.text, typeName.length);
    return isInstanceof(obj->type, obj->dimensions, typeName, 0);
}

bool Flint::isInstanceof(const FlintConstUtf8 &typeName1, uint32_t dimensions1, const FlintConstUtf8 &typeName2, uint32_t dimensions2) {
    if((dimensions1 >= dimensions2) && (typeName2 == objectClassName))
        return true;
    if(dimensions1 != dimensions2)
        return false;
    if(FlintJavaObject::isPrimType(typeName1) || FlintJavaObject::isPrimType(typeName2))
        return (typeName1.text[0] == typeName2.text[0]);
    const FlintConstUtf8 *objType = &typeName1;
    while(1) {
        if(*objType == typeName2)
            return true;
        FlintClassLoader &loader = load(*objType);
        uint16_t interfacesCount = loader.getInterfacesCount();
        for(uint32_t i = 0; i < interfacesCount; i++) {
            if(loader.getInterface(i) == typeName2)
                return true;
        }
        objType = loader.superClass;
        if(objType == NULL)
            return false;
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
