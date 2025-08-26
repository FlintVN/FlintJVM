
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_const_name_base_hash_table.h"

FlintMutex Flint::flintMutex;
Flint Flint::flintInstance;
uint32_t Flint::objectCount = 0;

FlintExecutionNode::FlintExecutionNode(Flint &flint, JThread *onwerThread, uint32_t stackSize) : FlintExecution(flint, onwerThread, stackSize) {
    prev = NULL_PTR;
    next = NULL_PTR;
}

void Flint::lock(void) {
    flintMutex.lock();
}

void Flint::unlock(void) {
    flintMutex.unlock();
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
    dbg = NULL_PTR;
    executionList = NULL_PTR;
    objectList = NULL_PTR;
    classArray0 = NULL_PTR;
    objectSizeToGc = 0;
}

FlintDebugger *Flint::getDebugger(void) const {
    return dbg;
}

void Flint::setDebugger(FlintDebugger *dbg) {
    this->dbg = dbg;
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

void Flint::print(const char *text) {
    uint32_t len = strlen(text);
    print(text, len, 0);
}

void Flint::print(FlintConstUtf8 &utf8) {
    print(utf8.text, utf8.length, 0);
}

void Flint::print(JString *str) {
    print(str->getText(), str->getLength(), str->getCoder());
}

void Flint::print(const char *text, uint32_t length, uint8_t coder) {
    if(dbg)
        dbg->print(text, length, coder);
    else
        FlintAPI::System::print(text, length, coder);
}

void Flint::println(int64_t num) {
    print(num);
    print("\n", 1, 0);
}

void Flint::println(const char *text) {
    uint32_t len = strlen(text);
    print(text, len, 0);
    print("\n", 1, 0);
}

void Flint::println(FlintConstUtf8 &utf8) {
    print(utf8.text, utf8.length, 0);
    print("\n", 1, 0);
}

void Flint::println(JString *str) {
    print(str->getText(), str->getLength(), str->getCoder());
    print("\n", 1, 0);
}

FlintResult<FlintExecution> Flint::newExecution(JThread *onwerThread, uint32_t stackSize) {
    FlintExecutionNode *newNode = (FlintExecutionNode *)Flint::malloc(sizeof(FlintExecutionNode));
    new (newNode)FlintExecutionNode(*this, onwerThread, stackSize);
    if(newNode == NULL_PTR)
        return ERR_OUT_OF_MEMORY;
    lock();
    newNode->next = executionList;
    if(executionList)
        executionList->prev = newNode;
    executionList = newNode;
    unlock();
    return newNode;
}

FlintResult<FlintExecution> Flint::getExcutionByThread(JThread *thread) const {
    Flint::lock();
    for(FlintExecutionNode *node = executionList; node != NULL_PTR; node = node->next) {
        if(node->onwerThread == thread) {
            Flint::unlock();
            return node;
        }
    }
    Flint::unlock();
    return FlintResult<FlintExecution>(NULL_PTR);
}

void Flint::freeExecution(FlintExecution *exec) {
    Flint::lock();
    FlintExecutionNode *prev = ((FlintExecutionNode *)exec)->prev;
    FlintExecutionNode *next = ((FlintExecutionNode *)exec)->next;
    if(prev)
        prev->next = next;
    else
        executionList = next;
    if(next)
        next->prev = prev;
    ((FlintExecutionNode *)exec)->~FlintExecutionNode();
    Flint::free(exec);
    Flint::unlock();
}

FlintResult<JObject> Flint::newObject(uint32_t size, FlintConstUtf8 *type, uint8_t dimensions) {
    objectSizeToGc += size;
    if(objectSizeToGc >= OBJECT_SIZE_TO_GC)
        garbageCollection();
    JObject *newNode = (JObject *)Flint::malloc(sizeof(JObject) + size);
    if(newNode == NULL_PTR)
        return ERR_OUT_OF_MEMORY;
    new (newNode)JObject(size, *type, dimensions);

    Flint::lock();
    newNode->prev = NULL_PTR;
    newNode->next = objectList;
    if(objectList)
        objectList->prev = newNode;
    objectList = newNode;
    Flint::unlock();

    return newNode;
}

FlintResult<JObject> Flint::newObject(FlintConstUtf8 *type) {
    auto obj = newObject(sizeof(FlintFieldsData), type, 0);
    if(obj.err == ERR_OK) {
        memset(obj.value->data, 0, sizeof(FlintFieldsData));

        auto loader = load(type->text);
        if(loader.err != ERR_OK) {
            freeObject(obj.value);
            return *(FlintResult<JObject> *)&loader;
        }

        /* init field data */
        FlintFieldsData *fields = (FlintFieldsData *)obj.value->data;
        new (fields)FlintFieldsData();
        auto res = fields->loadNonStatic(*this, *loader.value);
        if(res.err != ERR_OK)
            return FlintResult<JObject>(res.err, res.getErrorMsg(), res.getErrorMsgLength());
    }
    return obj;
}

FlintResult<JInt8Array> Flint::newBooleanArray(uint32_t length) {
    auto obj = newObject(length, (FlintConstUtf8 *)booleanPrimTypeName, 1);
    return *(FlintResult<JInt8Array> *)&obj;
}

FlintResult<JInt8Array> Flint::newByteArray(uint32_t length) {
    auto obj = newObject(length, (FlintConstUtf8 *)bytePrimTypeName, 1);
    return *(FlintResult<JInt8Array> *)&obj;
}

FlintResult<JInt16Array> Flint::newCharArray(uint32_t length) {
    auto obj = newObject(length * sizeof(int16_t), (FlintConstUtf8 *)charPrimTypeName, 1);
    return *(FlintResult<JInt16Array> *)&obj;
}

FlintResult<JInt16Array> Flint::newShortArray(uint32_t length) {
    auto obj = newObject(length * sizeof(int16_t), (FlintConstUtf8 *)shortPrimTypeName, 1);
    return *(FlintResult<JInt16Array> *)&obj;
}

FlintResult<JInt32Array> Flint::newIntegerArray(uint32_t length) {
    auto obj = newObject(length * sizeof(int32_t), (FlintConstUtf8 *)integerPrimTypeName, 1);
    return *(FlintResult<JInt32Array> *)&obj;
}

FlintResult<JFloatArray> Flint::newFloatArray(uint32_t length) {
    auto obj = newObject(length * sizeof(float), (FlintConstUtf8 *)floatPrimTypeName, 1);
    return *(FlintResult<JFloatArray> *)&obj;
}

FlintResult<JInt64Array> Flint::newLongArray(uint32_t length) {
    auto obj = newObject(length * sizeof(int64_t), (FlintConstUtf8 *)longPrimTypeName, 1);
    return *(FlintResult<JInt64Array> *)&obj;
}

FlintResult<JDoubleArray> Flint::newDoubleArray(uint32_t length) {
    auto obj = newObject(length * sizeof(double), (FlintConstUtf8 *)doublePrimTypeName, 1);
    return *(FlintResult<JDoubleArray> *)&obj;
}

FlintResult<JObjectArray> Flint::newObjectArray(FlintConstUtf8 *type, uint32_t length) {
    auto obj = newObject(length * sizeof(JObject *), type, 1);
    return *(FlintResult<JObjectArray> *)&obj;
}

FlintResult<JObject> Flint::newMultiArray(FlintConstUtf8 *typeName, int32_t *counts, uint8_t startDims, uint8_t endDims) {
    if(startDims > 1) {
        auto array = newObject(counts[0] * sizeof(JObject *), typeName, startDims);
        if(array.err == ERR_OK) {
            if(startDims > endDims) {
                for(uint32_t i = 0; i < counts[0]; i++) {
                    auto subArray = newMultiArray(typeName, &counts[1], startDims - 1, endDims);
                    if(subArray.err != ERR_OK) {
                        for(uint32_t j = 0; j < i; j++)
                            freeObject(((JObject **)(array.value->data))[j]);
                        return subArray;
                    }
                    ((JObject **)(array.value->data))[i] = subArray.value;
                }
            }
            else
                memset(array.value->data, 0, array.value->size);
        }
        return array;
    }
    else {
        uint8_t atype = JObject::isPrimType(*typeName);
        uint8_t typeSize = atype ? JObject::getPrimitiveTypeSize(atype) : sizeof(JObject *);
        auto array = newObject(typeSize * counts[0], typeName, 1);
        if(array.err == ERR_OK)
            memset(array.value->data, 0, array.value->size);
        return array;
    }
}

FlintResult<JClass> Flint::newClass(JString *typeName) {
    // TODO - Check the existence of type

    auto cls = newObject((FlintConstUtf8 *)classClassName);

    /* set value for name field */
    if(cls.err == ERR_OK)
        ((JClass *)cls.value)->setName(typeName);

    return *(FlintResult<JClass> *)&cls;
}

FlintResult<JClass> Flint::newClass(const char *typeName, uint16_t length) {
    /* create String object to store typeName */
    auto name = newString(typeName, length, false);
    if(name.err != ERR_OK)
        return *(FlintResult<JClass> *)&name;

    /* replace '/' to '.' */
    char *text = (char *)name.value->getText();
    for(uint32_t i = 0; i < length; i++) {
        if(text[i] == '/')
            text[i] = '.';
    }
    auto cls = newClass(name.value);
    if(cls.err != ERR_OK)
        freeObject(name.value);
    return cls;
}

FlintResult<JClass> Flint::getConstClass(const char *typeName, uint16_t length) {
    if(*typeName == 'L') {
        length -= (typeName[length - 1] == ';') ? 2 : 1;
        typeName++;
    }

    Flint::lock();
    JClass *cls = constClassTree.find(typeName, length);
    if(!cls) {
        auto newCls = newClass(typeName, length);
        if(newCls.err == ERR_OK)
            constClassTree.add(newCls.value);
        Flint::unlock();
        return newCls;
    }
    Flint::unlock();

    return cls;
}

FlintResult<JClass> Flint::getConstClass(JString *str) {
    Flint::lock();
    JClass *cls = constClassTree.find(str);
    if(!cls) {
        auto newCls = newClass(str);
        if(newCls.err == ERR_OK)
            constClassTree.add(newCls.value);
        Flint::unlock();
        return newCls;
    }
    Flint::unlock();

    return cls;
}

FlintResult<JString> Flint::newString(uint16_t length, uint8_t coder) {
    /* create new byte array to store string */
    auto byteArray = newByteArray(length << (coder ? 1 : 0));
    if(byteArray.err != ERR_OK)
        return *(FlintResult<JString> *)&byteArray;

    /* create new string object */
    auto str = newObject((FlintConstUtf8 *)stringClassName);
    if(str.err != ERR_OK)
        freeObject(byteArray.value);
    else {
        /* set value for value field */
        ((JString *)str.value)->setValue(*byteArray.value);
        /* set value for coder field */
        ((JString *)str.value)->setCoder(coder);
    }

    return *(FlintResult<JString> *)&str;
}

FlintResult<JString> Flint::newString(const char *text) {
    uint32_t len = strlen(text);
    return newString(text, len, false);
}

FlintResult<JString> Flint::newString(const char *text, uint16_t size, bool isUtf8) {
    uint32_t index = 0;
    bool isLatin1 = isUtf8 ? JString::isLatin1(text) : true;
    uint32_t strLen = isUtf8 ? JString::utf8StrLen(text) : size;

    /* create new byte array to store string */
    uint32_t arrayLen = isLatin1 ? strLen : (strLen << 1);
    auto byteArray = newByteArray(arrayLen);
    if(byteArray.err != ERR_OK)
        return *(FlintResult<JString> *)&byteArray;
    uint8_t *byteArrayData = (uint8_t *)byteArray.value->getData();
    if(!isUtf8)
        memcpy(byteArrayData, text, strLen);
    else {
        if(isLatin1) {
            while(*text) {
                uint32_t c = JString::utf8Decode(text);
                byteArrayData[index] = c;
                text += JString::getUtf8DecodeSize(*text);
                index++;
            }
        }
        else while(*text) {
            uint32_t c = JString::utf8Decode(text);
            ((uint16_t *)byteArrayData)[index] = c;
            text += JString::getUtf8DecodeSize(*text);
            index++;
        }
    }

    /* create new string object */
    auto str = newObject((FlintConstUtf8 *)stringClassName);
    if(str.err != ERR_OK)
        freeObject(byteArray.value);
    else {
        /* set value for value field */
        ((JString *)str.value)->setValue(*byteArray.value);
        /* set value for coder field */
        ((JString *)str.value)->setCoder(isLatin1 ? 0 : 1);
    }

    return *(FlintResult<JString> *)&str;
}

FlintResult<JString> Flint::getConstString(FlintConstUtf8 &utf8) {
    Flint::lock();
    JString *str = constStringTree.find(utf8);
    if(!str) {
        auto newStr = newString(utf8.text, utf8.length, true);
        if(newStr.err == ERR_OK)
            newStr = constStringTree.add(newStr.value);
        Flint::unlock();
        return newStr;
    }
    Flint::unlock();
    return str;
}

FlintResult<JString> Flint::getConstString(JString *str) {
    Flint::lock();
    JString *ret = constStringTree.find(str);
    if(!ret) {
        auto tmp = constStringTree.add(str);
        Flint::unlock();
        return tmp;
    }
    Flint::unlock();
    return ret;
}

static FlintConstUtf8 *getConstUtf8InBaseConstName(const char *text, uint32_t hash, bool isTypeName) {
    uint32_t hashIndex = Flint_HashIndex(hash, LENGTH(baseConstUtf8HashTable));
    if(baseConstUtf8HashTable[hashIndex]) {
        int32_t left = 0;
        int32_t right = baseConstUtf8HashTable[hashIndex]->count - 1;
        FlintConstUtf8 * const *constUtf8List = baseConstUtf8HashTable[hashIndex]->values;
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
    return NULL_PTR;
}

FlintResult<FlintConstUtf8> Flint::getConstUtf8(const char *text, uint16_t length) {
    uint32_t hash = Flint_CalcHash(text, length, false);
    FlintConstUtf8 *ret = getConstUtf8InBaseConstName(text, hash, false);
    if(ret)
        return ret;

    Flint::lock();
    ret = constUtf8Tree.find(text, hash, false);
    if(!ret) {
        auto utf8 = constUtf8Tree.add(text, hash, false);
        if(utf8.err != ERR_OK) {
            Flint::unlock();
            return *(FlintResult<FlintConstUtf8> *)&utf8;
        }
        ret = utf8.value;
    }
    Flint::unlock();

    return ret;
}

FlintResult<FlintConstUtf8> Flint::getTypeNameConstUtf8(const char *typeName, uint16_t length) {
    uint32_t hash = Flint_CalcHash(typeName, length, true);
    FlintConstUtf8 *ret = getConstUtf8InBaseConstName(typeName, hash, true);
    if(ret)
        return ret;

    Flint::lock();
    ret = constUtf8Tree.find(typeName, hash, true);
    if(!ret) {
        auto utf8 = constUtf8Tree.add(typeName, hash, true);
        if(utf8.err != ERR_OK) {
            Flint::unlock();
            return *(FlintResult<FlintConstUtf8> *)&utf8;
        }
        ret = utf8.value;
    }
    Flint::unlock();

    return ret;
}

FlintResult<JObjectArray> Flint::getClassArray0(void) {
    if(classArray0)
        return classArray0;
    Flint::lock();
    if(classArray0 == NULL_PTR) {
        auto obj = newObjectArray((FlintConstUtf8 *)classClassName, 0);
        if(obj.err == ERR_OK)
            classArray0 = obj.value;
        Flint::unlock();
        return obj;
    }
    else {
        Flint::unlock();
        return classArray0;
    }
}

FlintResult<JThrowable> Flint::newThrowable(JString *str, FlintConstUtf8 *excpType) {
    /* create new exception object */
    auto excp = newObject(excpType);

    /* set detailMessage value */
    if(excp.err == ERR_OK && str)
        ((JThrowable *)excp.value)->setDetailMessage(str);

    return *(FlintResult<JThrowable> *)&excp;
}

FlintResult<FlintJavaBoolean> Flint::newBoolean(bool value) {
    auto obj = newObject((FlintConstUtf8 *)booleanClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaBoolean *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaBoolean> *)&obj;
}

FlintResult<FlintJavaByte> Flint::newByte(int8_t value) {
    auto obj = newObject((FlintConstUtf8 *)byteClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaByte *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaByte> *)&obj;
}

FlintResult<FlintJavaCharacter> Flint::newCharacter(uint16_t value) {
    auto obj = newObject((FlintConstUtf8 *)characterClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaCharacter *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaCharacter> *)&obj;
}

FlintResult<FlintJavaShort> Flint::newShort(int16_t value) {
    auto obj = newObject((FlintConstUtf8 *)shortClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaShort *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaShort> *)&obj;
}

FlintResult<FlintJavaInteger> Flint::newInteger(int32_t value) {
    auto obj = newObject((FlintConstUtf8 *)integerClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaInteger *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaInteger> *)&obj;
}

FlintResult<FlintJavaFloat> Flint::newFloat(float value) {
    auto obj = newObject((FlintConstUtf8 *)floatClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaFloat *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaFloat> *)&obj;
}

FlintResult<FlintJavaLong> Flint::newLong(int64_t value) {
    auto obj = newObject((FlintConstUtf8 *)longClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaLong *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaLong> *)&obj;
}

FlintResult<FlintJavaDouble> Flint::newDouble(double value) {
    auto obj = newObject((FlintConstUtf8 *)doubleClassName);
    if(obj.err == ERR_OK)
        ((FlintJavaDouble *)obj.value)->setValue(value);
    return *(FlintResult<FlintJavaDouble> *)&obj;
}

void Flint::garbageCollectionProtectObject(JObject *obj) {
    bool isPrim = JObject::isPrimType(obj->type);
    obj->setProtected();
    if((obj->dimensions > 1) || (obj->dimensions == 1 && !isPrim)) {
        uint32_t count = obj->size / 4;
        for(uint32_t i = 0; i < count; i++) {
            JObject *tmp = ((JObject **)obj->data)[i];
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(tmp);
        }
    }
    else if(!isPrim) {
        FlintFieldsData &fieldData = *(FlintFieldsData *)obj->data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            JObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && !tmp->getProtected())
                garbageCollectionProtectObject(tmp);
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
    JObject *obj = (JObject *)address;
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

void Flint::clearProtectObjectNew(JObject *obj) {
    bool isPrim = JObject::isPrimType(obj->type);
    obj->clearProtected();
    if((obj->dimensions > 1) || (obj->dimensions == 1 && !isPrim)) {
        uint32_t count = obj->size / 4;
        for(uint32_t i = 0; i < count; i++) {
            JObject *tmp = ((JObject **)obj->data)[i];
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(tmp);
        }
    }
    else if(!isPrim) {
        FlintFieldsData &fieldData = *(FlintFieldsData *)obj->data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            JObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && (tmp->getProtected() & 0x02))
                clearProtectObjectNew(tmp);
        }
    }
}

void Flint::garbageCollection(void) {
    Flint::lock();
    objectSizeToGc = 0;
    constClassTree.forEach([](JClass *item) {
        if(!item->getProtected())
            garbageCollectionProtectObject(item);
    });
    constStringTree.forEach([](JString *item) {
        if(!item->getProtected())
            garbageCollectionProtectObject(item);
    });
    classDataTree.forEach([](FlintClassData *item) {
        FlintFieldsData *fieldsData = item->staticFieldsData;
        if(fieldsData && fieldsData->fieldsObjCount) {
            for(uint32_t i = 0; i < fieldsData->fieldsObjCount; i++) {
                JObject *obj = fieldsData->fieldsObject[i].object;
                if(obj && !obj->getProtected())
                    garbageCollectionProtectObject(obj);
            }
        }
    });
    for(FlintExecutionNode *node = executionList; node != NULL_PTR; node = node->next) {
        if(node->onwerThread && !node->onwerThread->getProtected())
            garbageCollectionProtectObject(node->onwerThread);
        int32_t startSp = node->startSp;
        int32_t endSp = FLINT_MAX(node->sp, node->peakSp);
        while(startSp >= 3) {
            for(int32_t i = startSp; i <= endSp; i++) {
                if(isObject(node->stack[i])) {
                    JObject *obj = (JObject *)node->stack[i];
                    if(obj && !obj->getProtected())
                        garbageCollectionProtectObject(obj);
                }
            }
            endSp = startSp - 4;
            startSp = node->stack[startSp];
        }
    }
    if(classArray0 && !((JObjectArray *)classArray0)->getProtected())
        garbageCollectionProtectObject((JObject *)classArray0);
    for(JObject *node = objectList; node != NULL_PTR;) {
        JObject *next = node->next;
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

FlintResult<FlintClassData> Flint::createFlintClassData(const char *className, uint16_t length) {
    FlintClassData *classData = (FlintClassData *)Flint::malloc(sizeof(FlintClassData));
    if(classData == NULL_PTR)
        return ERR_OUT_OF_MEMORY;
    classData->staticFieldsData = NULL_PTR;
    new (classData)FlintClassData(*this);
    FlintError err = classData->load(className, length);
    if(err != ERR_OK)
        return FlintResult<FlintClassData>(err, className, length);
    return classData;
}

FlintResult<FlintClassLoader> Flint::load(const char *className, uint16_t length) {
    if(length == 0)
        length = strlen(className);
    Flint::lock();
    FlintClassLoader *loader = (FlintClassLoader *)classDataTree.find(className, length);
    if(!loader) {
        auto newLoader = createFlintClassData(className, length);
        if(newLoader.err == ERR_OK)
            classDataTree.add(*newLoader.value);
        Flint::unlock();
        return *(FlintResult<FlintClassLoader> *)&newLoader;
    }
    Flint::unlock();
    return loader;
}

FlintError Flint::initStaticField(FlintClassData *classData) {
    FlintFieldsData *fieldsData = (FlintFieldsData *)Flint::malloc(sizeof(FlintFieldsData));
    if(fieldsData == NULL_PTR)
        return ERR_OUT_OF_MEMORY;
    new (fieldsData)FlintFieldsData();
    FlintError err = fieldsData->loadStatic(*classData).err;
    if(err != ERR_OK)
        return err;
    classData->staticFieldsData = fieldsData;
    return ERR_OK;
}

FlintResult<FlintMethodInfo> Flint::findMethod(FlintConstMethod &constMethod) {
    auto loader = load(constMethod.className.text);
    if(loader.err != ERR_OK)
        return *(FlintResult<FlintMethodInfo> *)&loader;
    while(loader.err == ERR_OK) {
        auto methodInfo = loader.value->getMethodInfo(constMethod.nameAndType);
        if(methodInfo.err != ERR_METHOD_NOT_FOUND)
            return methodInfo;
        FlintConstUtf8 *superClass = loader.value->superClass;
        if(!superClass)
            return ERR_METHOD_NOT_FOUND;
        loader = load(superClass->text);
    }
    return *(FlintResult<FlintMethodInfo> *)&loader;
}

FlintResult<FlintMethodInfo> Flint::findMethod(FlintConstUtf8 &className, FlintConstNameAndType &nameAndType) {
    auto loader = load(className.text);
    while(loader.err == ERR_OK) {
        auto methodInfo = loader.value->getMethodInfo(nameAndType);
        if(methodInfo.err != ERR_METHOD_NOT_FOUND)
            return methodInfo;
        FlintConstUtf8 *superClass = loader.value->superClass;
        if(!superClass)
            return ERR_METHOD_NOT_FOUND;
        loader = load(superClass->text);
    }
    return *(FlintResult<FlintMethodInfo> *)&loader;
}

static bool compareClassName(FlintConstUtf8 &className1, const char *className2, uint32_t hash) {
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

FlintResult<bool> Flint::isInstanceof(JObject *obj, const char *typeName, uint16_t length) {
    uint32_t dimensions = getDimensions(typeName);
    if(length == 0)
        length = strlen(typeName);
    typeName += dimensions;
    length -= dimensions;
    if(*typeName == 'L') {
        length -= (typeName[length - 1] == ';') ? 2 : 1;
        typeName++;
    }
    uint32_t typeNameHash = Flint_CalcHash(typeName, length, true);
    if((obj->dimensions >= dimensions) && compareClassName(*(FlintConstUtf8 *)objectClassName, typeName, typeNameHash))
        return true;
    if(dimensions != obj->dimensions)
        return false;
    FlintConstUtf8 *objType = &obj->type;
    if(JObject::isPrimType(*objType) || ((length == 1) && JObject::convertToAType(typeName[0])))
        return (length == objType->length) && (typeName[0] == objType->text[0]);
    while(1) {
        if(compareClassName(*objType, typeName, typeNameHash))
            return true;
        auto loader = load(objType->text);
        if(loader.err != ERR_OK)
            return *(FlintResult<bool> *)&loader;
        uint16_t interfacesCount = loader.value->getInterfacesCount();
        for(uint32_t i = 0; i < interfacesCount; i++) {
            if(compareClassName(loader.value->getInterface(i), typeName, typeNameHash))
                return true;
        }
        objType = loader.value->superClass;
        if(objType == NULL_PTR)
            return false;
    }
}

FlintResult<bool> Flint::isInstanceof(FlintConstUtf8 &typeName1, uint32_t dimensions1, FlintConstUtf8 &typeName2, uint32_t dimensions2) {
    if((dimensions1 >= dimensions2) && (typeName2 == *(FlintConstUtf8 *)objectClassName))
        return ERR_OK;
    if(dimensions1 != dimensions2)
        return false;
    if(JObject::isPrimType(typeName1) || JObject::isPrimType(typeName2))
        return typeName1.text[0] == typeName2.text[0];
    FlintConstUtf8 *objType = &typeName1;
    while(1) {
        if(*objType == typeName2)
            return true;
        auto loader = load(objType->text);
        if(loader.err != ERR_OK)
            return *(FlintResult<bool> *)&loader;
        uint16_t interfacesCount = loader.value->getInterfacesCount();
        for(uint32_t i = 0; i < interfacesCount; i++) {
            if(loader.value->getInterface(i) == typeName2)
                return true;
        }
        objType = loader.value->superClass;
        if(objType == NULL_PTR)
            return false;
    }
}

FlintError Flint::runToMain(const char *mainClass, uint32_t stackSize) {
    auto loader = load(mainClass);
    RETURN_IF_ERR(loader.err);
    auto mainMethodInfo = loader.value->getMainMethodInfo();
    RETURN_IF_ERR(mainMethodInfo.err);
    auto exec = newExecution(NULL_PTR, stackSize);
    RETURN_IF_ERR(exec.err);
    return exec.value->run(mainMethodInfo.value) ? ERR_OK : ERR_OUT_OF_MEMORY;
}

bool Flint::isRunning(void) const {
    return executionList ? true : false;
}

void Flint::stopRequest(void) {
    Flint::lock();
    for(FlintExecutionNode *node = executionList; node != NULL_PTR; node = node->next)
        node->stopRequest();
    Flint::unlock();
}

void Flint::terminateRequest(void) {
    Flint::lock();
    for(FlintExecutionNode *node = executionList; node != NULL_PTR; node = node->next)
        node->terminateRequest();
    Flint::unlock();
}

void Flint::terminate(void) {
    terminateRequest();
    while(isRunning())
        FlintAPI::Thread::sleep(1);
}

void Flint::freeObject(JObject *obj) {
    Flint::lock();
    if(obj->prev)
        obj->prev->next = obj->next;
    else
        objectList = obj->next;
    if(obj->next)
        obj->next->prev = obj->prev;

    if(obj->dimensions == 0)
        obj->getFields().~FlintFieldsData();
    Flint::free(obj);
    Flint::unlock();
}

void Flint::clearAllStaticFields(void) {
    classDataTree.forEach([](FlintClassData *item) {
        item->staticInitOwnId = 0;
        item->clearStaticFields();
    });
}

void Flint::freeAllObject(void) {
    Flint::lock();
    constClassTree.clear();
    constStringTree.clear();
    for(JObject *node = objectList; node != NULL_PTR;) {
        JObject *next = node->next;
        if(node->dimensions == 0)
            node->getFields().~FlintFieldsData();
        Flint::free(node);
        node = next;
    }
    objectList = NULL_PTR;
    classArray0 = NULL_PTR;
    objectSizeToGc = 0;
    Flint::unlock();
}

void Flint::freeAllExecution(void) {
    Flint::lock();
    for(FlintExecutionNode *node = executionList; node != NULL_PTR;) {
        FlintExecutionNode *next = node->next;
        node->~FlintExecutionNode();
        Flint::free(node);
        node = next;
    }
    executionList = NULL_PTR;
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
