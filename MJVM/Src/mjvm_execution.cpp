
#include <string.h>
#include "mjvm.h"
#include "mjvm_opcodes.h"
#include "mjvm_execution.h"
#include "mjvm_const_name.h"

#define FLOAT_NAN                   0x7FC00000
#define DOUBLE_NAN                  0x7FF8000000000000

#define ARRAY_TO_INT16(array)       (int16_t)(((array)[0] << 8) | (array)[1])
#define ARRAY_TO_INT32(array)       (int32_t)(((array)[0] << 24) | ((array)[1] << 16) | ((array)[2] << 8) | (array)[3])

static const uint8_t primitiveTypeSize[8] = {
    sizeof(int8_t), sizeof(int16_t), sizeof(float), sizeof(double),
    sizeof(int8_t), sizeof(int16_t), sizeof(int32_t), sizeof(int64_t)
};

Execution::Execution(void) : stackLength(DEFAULT_STACK_SIZE / sizeof(int32_t)) {
    lr = -1;
    sp = -1;
    startSp = sp;
    peakSp = sp;
    stack = (int32_t *)Mjvm::malloc(DEFAULT_STACK_SIZE);
    stackType = (uint8_t *)Mjvm::malloc(DEFAULT_STACK_SIZE / sizeof(int32_t) / 8);
    staticClassDataList = 0;
    objectList = 0;
    constStringList = 0;
    objectCountToGc = 0;
}

Execution::Execution(uint32_t size) : stackLength(size / sizeof(int32_t)) {
    lr = -1;
    sp = -1;
    startSp = sp;
    peakSp = sp;
    stack = (int32_t *)Mjvm::malloc(size);
    stackType = (uint8_t *)Mjvm::malloc(size / sizeof(int32_t) / 8);
    staticClassDataList = 0;
    objectList = 0;
    constStringList = 0;
    objectCountToGc = 0;
}

void Execution::addToList(MjvmObjectNode **list, MjvmObjectNode *objNode) {
    objNode->prev = 0;
    objNode->next = *list;
    if(*list)
        (*list)->prev = objNode;
    *list = objNode;
}

void Execution::removeFromList(MjvmObjectNode **list, MjvmObjectNode *objNode) {
    if(objNode->prev)
        objNode->prev->next = objNode->next;
    else
        *list = objNode->next;
    if(objNode->next)
        objNode->next->prev = objNode->prev;
}

MjvmObject *Execution::newObject(uint32_t size, const ConstUtf8 &type, uint8_t dimensions) {
    if(objectCountToGc++ >= NUM_OBJECT_TO_GC)
        garbageCollection();
    MjvmObjectNode *newNode = (MjvmObjectNode *)Mjvm::malloc(sizeof(MjvmObjectNode) + sizeof(MjvmObject) + size);
    MjvmObject *ret = newNode->getMjvmObject();
    new (ret)MjvmObject(size, type, dimensions);
    addToList(&objectList, newNode);
    return ret;
}

MjvmObjectNode *Execution::newStringNode(const char *str, uint16_t length) {
    /* create new byte array to store string */
    MjvmObject *byteArray = (MjvmObject *)Mjvm::malloc(sizeof(MjvmObject) + length);
    new (byteArray)MjvmObject(length, *primTypeConstUtf8List[4], 1);
    memcpy(byteArray->data, str, length);

    /* create new string object */
    MjvmObjectNode *strObjNode = (MjvmObjectNode *)Mjvm::malloc(sizeof(MjvmObjectNode) + sizeof(MjvmObject) + sizeof(FieldsData));
    MjvmObject *strObj = strObjNode->getMjvmObject();
    new (strObj)MjvmObject(sizeof(FieldsData), stringClassName, 0);

    /* init field data */
    FieldsData *fields = (FieldsData *)strObj->data;
    new (fields)FieldsData(*this, load(stringClassName), false);

    /* set value for value field */
    fields->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object = byteArray;

    return strObjNode;
}

MjvmObjectNode *Execution::newStringNode(const char *str[], uint16_t count) {
    uint16_t index = 0;
    uint16_t length = 0;
    for(uint16_t i = 0; i < count; i++)
        length += strlen(str[i]);

    /* create new byte array to store string */
    MjvmObject *byteArray = (MjvmObject *)Mjvm::malloc(sizeof(MjvmObject) + length);
    new (byteArray)MjvmObject(length, *primTypeConstUtf8List[4], 1);
    for(uint16_t i = 0; i < count; i++) {
        const char *buff = str[i];
        while(*buff) {
            byteArray->data[index] = *buff;
            buff++;
            index++;
        }
    }

    /* create new string object */
    MjvmObjectNode *strObjNode = (MjvmObjectNode *)Mjvm::malloc(sizeof(MjvmObjectNode) + sizeof(MjvmObject) + sizeof(FieldsData));
    MjvmObject *strObj = strObjNode->getMjvmObject();
    new (strObj)MjvmObject(sizeof(FieldsData), stringClassName, 0);

    /* init field data */
    FieldsData *fields = (FieldsData *)strObj->data;
    new (fields)FieldsData(*this, load(stringClassName), false);

    /* set value for value field */
    fields->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object = byteArray;

    return strObjNode;
}

MjvmObject *Execution::newString(const char *str) {
    MjvmObjectNode *strObjNode = newStringNode(str, strlen(str));
    addToList(&objectList, strObjNode);
    return strObjNode->getMjvmObject();
}

MjvmObject *Execution::newString(const char *str, uint16_t length) {
    MjvmObjectNode *strObjNode = newStringNode(str, length);
    addToList(&objectList, strObjNode);
    return strObjNode->getMjvmObject();
}

MjvmObject *Execution::newString(const char *str[], uint16_t count) {
    MjvmObjectNode *strObjNode = newStringNode(str, count);
    addToList(&objectList, strObjNode);
    return strObjNode->getMjvmObject();
}

MjvmObject *Execution::getConstString(const ConstUtf8 &str) {
    for(MjvmObjectNode *node = constStringList; node != 0; node = node->next) {
        MjvmObject *strObj = node->getMjvmObject();
        MjvmObject *value = ((FieldsData *)strObj->data)->getFieldObject(*(ConstNameAndType *)stringValueFieldName).object;
        uint32_t length = value->size / sizeof(int8_t);
        if(length == str.length && strncmp(str.getText(), (char *)value->data, length) == 0)
            return strObj;
    }
    MjvmObjectNode *newNode = newStringNode(str.getText(), str.length);
    addToList(&constStringList, newNode);
    return newNode->getMjvmObject();
}

MjvmObject *Execution::newNullPointerException(MjvmObject *strObj) {
    /* create new NullPointerException object */
    MjvmObject *obj = newObject(sizeof(FieldsData), nullPtrExcpClassName);

    /* init field data */
    FieldsData *fields = (FieldsData *)obj->data;
    new (fields)FieldsData(*this, load(nullPtrExcpClassName), false);

    /* set detailMessage value */
    fields->getFieldObject(*(ConstNameAndType *)exceptionDetailMessageFieldName).object = strObj;

    return obj;
}

static uint8_t convertToAType(char type) {
    switch(type) {
        case 'Z':
            return 4;
        case 'C':
            return 5;
        case 'F':
            return 6;
        case 'D':
            return 7;
        case 'B':
            return 8;
        case 'S':
            return 8;
        case 'I':
            return 10;
        case 'J':
            return 11;
    }
    return 0;
}

static uint8_t isPrimType(const ConstUtf8 &type) {
    if(type.length == 1)
        convertToAType(type.getText()[0]);
    return 0;
}

void Execution::freeAllObject(void) {
    for(MjvmObjectNode *node = constStringList; node != 0;) {
        MjvmObjectNode *next = node->next;
        MjvmObject *obj = node->getMjvmObject();
        if(obj->dimensions == 0) {
            FieldsData *fields = (FieldsData *)obj->data;
            fields->~FieldsData();
        }
        Mjvm::free(node);
        node = next;
    }
    for(MjvmObjectNode *node = objectList; node != 0;) {
        MjvmObjectNode *next = node->next;
        MjvmObject *obj = node->getMjvmObject();
        if(obj->dimensions == 0) {
            FieldsData *fields = (FieldsData *)obj->data;
            fields->~FieldsData();
        }
        Mjvm::free(node);
        node = next;
    }
    objectList = 0;
}

void Execution::garbageCollectionProtectObject(MjvmObject *obj) {
    bool isPrim = isPrimType(obj->type);
    if((obj->dimensions > 1) || (obj->dimensions == 1 && !isPrim)) {
        uint32_t count = obj->size / 4;
        for(uint32_t i = 0; i < count; i++) {
            MjvmObject *tmp = ((MjvmObject **)obj->data)[i];
            if(tmp && !tmp->isProtected())
                garbageCollectionProtectObject(tmp);
        }
    }
    else if(!isPrim) {
        FieldsData &fieldData = *(FieldsData *)obj->data;
        for(uint16_t i = 0; i < fieldData.fieldsObjCount; i++) {
            MjvmObject *tmp = fieldData.fieldsObject[i].object;
            if(tmp && !tmp->isProtected())
                garbageCollectionProtectObject(tmp);
        }
    }
    obj->setProtected();
}

void Execution::garbageCollection(void) {
    objectCountToGc = 0;
    for(ClassDataNode *node = staticClassDataList; node != 0; node = node->next) {
        FieldsData *fieldsData = node->filedsData;
        if(fieldsData->fieldsObjCount) {
            for(uint32_t i = 0; i < fieldsData->fieldsObjCount; i++) {
                MjvmObject *obj = fieldsData->fieldsObject[i].object;
                if(obj && !obj->isProtected())
                    garbageCollectionProtectObject(obj);
            }
        }
    }
    for(int32_t i = 0; i <= peakSp; i++) {
        if(getStackType(i) == STACK_TYPE_OBJECT) {
            MjvmObject *obj = (MjvmObject *)stack[i];
            if(obj && !obj->isProtected())
                garbageCollectionProtectObject(obj);
        }
    }
    for(MjvmObjectNode *node = objectList; node != 0;) {
        MjvmObjectNode *next = node->next;
        MjvmObject *obj = node->getMjvmObject();
        if(!obj->isProtected()) {
            removeFromList(&objectList, node);
            if(obj->dimensions == 0) {
                FieldsData *fields = (FieldsData *)obj->data;
                fields->~FieldsData();
            }
            Mjvm::free(node);
        }
        node = next;
    }
}

const ClassLoader &Execution::load(const char *className) {
    return load(className, strlen(className));
}

const ClassLoader &Execution::load(const char *className, uint16_t length) {
    for(ClassDataNode *node = staticClassDataList; node != 0; node = node->next) {
        const ConstUtf8 &name = node->classLoader.getThisClass();
        if(name.length == length && strncmp(name.getText(), className, length) == 0)
            return node->classLoader;
    }
    ClassDataNode *newNode = (ClassDataNode *)Mjvm::malloc(sizeof(ClassDataNode));
    new (newNode)ClassDataNode(Mjvm::load(className, length), 0);
    newNode->next = staticClassDataList;
    staticClassDataList = newNode;
    return newNode->classLoader;
}

const ClassLoader &Execution::load(const ConstUtf8 &className) {
    for(ClassDataNode *node = staticClassDataList; node != 0; node = node->next) {
        const ConstUtf8 &name = node->classLoader.getThisClass();
        if(node->classLoader.getThisClass() == className)
            return node->classLoader;
    }
    ClassDataNode *newNode = (ClassDataNode *)Mjvm::malloc(sizeof(ClassDataNode));
    new (newNode)ClassDataNode(Mjvm::load(className), 0);
    newNode->next = staticClassDataList;
    staticClassDataList = newNode;
    return newNode->classLoader;
}

const FieldsData &Execution::getStaticFields(const ConstUtf8 &className) const {
    for(ClassDataNode *node = staticClassDataList; node != 0; node = node->next) {
        if(className == node->classLoader.getThisClass())
            return *node->filedsData;
    }
    return *(const FieldsData *)0;
}

const FieldsData &Execution::getStaticFields(const ClassLoader &classLoader) const {
    for(ClassDataNode *node = staticClassDataList; node != 0; node = node->next) {
        if(&classLoader == &node->classLoader)
            return *node->filedsData;
    }
    return *(const FieldsData *)0;
}

void Execution::initStaticField(const ClassLoader &classLoader) {
    ClassDataNode *newNode = (ClassDataNode *)Mjvm::malloc(sizeof(ClassDataNode));
    FieldsData *fieldsData = (FieldsData *)Mjvm::malloc(sizeof(FieldsData));
    new (fieldsData)FieldsData(*this, classLoader, true);
    new (newNode)ClassDataNode(classLoader, fieldsData);
    newNode->next = staticClassDataList;
    staticClassDataList = newNode;
}

Execution::StackType Execution::getStackType(uint32_t index) {
    return (stackType[index / 8] & (1 << (index % 8))) ? STACK_TYPE_OBJECT : STACK_TYPE_NON_OBJECT;
}

Execution::StackValue Execution::getStackValue(uint32_t index) {
    StackValue ret = {
        .type = (stackType[index / 8] & (1 << (index % 8))) ? STACK_TYPE_OBJECT : STACK_TYPE_NON_OBJECT,
        .value = stack[index],
    };
    return ret;
}

void Execution::setStackValue(uint32_t index, const StackValue &value) {
    stack[index] = value.value;
    if(value.type == STACK_TYPE_OBJECT)
        stackType[index / 8] |= (1 << (index % 8));
    else
        stackType[index / 8] &= ~(1 << (index % 8));
}

void Execution::stackPush(const StackValue &value) {
    sp = peakSp = sp + 1;
    stack[sp] = value.value;
    if(value.type == STACK_TYPE_OBJECT)
        stackType[sp / 8] |= (1 << (sp % 8));
    else
        stackType[sp / 8] &= ~(1 << (sp % 8));
}

void Execution::stackPushInt32(int32_t value) {
    sp = peakSp = sp + 1;
    stack[sp] = value;
    stackType[sp / 8] &= ~(1 << (sp % 8));
}

void Execution::stackPushInt64(int64_t value) {
    if((sp + 2) < stackLength) {
        sp = peakSp = sp + 1;
        stack[sp] = ((uint32_t *)&value)[0];
        stackType[sp / 8] &= ~(1 << (sp % 8));
        sp = peakSp = sp + 1;
        stack[sp] = ((uint32_t *)&value)[1];
        stackType[sp / 8] &= ~(1 << (sp % 8));
    }
    else
        throw "stack overflow";
}

void Execution::stackPushFloat(float value) {
    sp = peakSp = sp + 1;
    stack[sp] = *(uint32_t *)&value;
    stackType[sp / 8] &= ~(1 << (sp % 8));
}

void Execution::stackPushDouble(double value) {
    if((sp + 2) < stackLength) {
        sp = peakSp = sp + 1;
        stack[sp] = ((uint32_t *)&value)[0];
        stackType[sp / 8] &= ~(1 << (sp % 8));
        sp = peakSp = sp + 1;
        stack[sp] = ((uint32_t *)&value)[1];
        stackType[sp / 8] &= ~(1 << (sp % 8));
    }
}

void Execution::stackPushObject(MjvmObject *obj) {
    sp = peakSp = sp + 1;
    stack[sp] = (int32_t)obj;
    stackType[sp / 8] |= (1 << (sp % 8));
}

int32_t Execution::stackPopInt32(void) {
    return stack[sp--];
}

int64_t Execution::stackPopInt64(void) {
    uint64_t ret;
    sp -= 2;
    ((uint32_t *)&ret)[0] = stack[sp + 1];
    ((uint32_t *)&ret)[1] = stack[sp + 2];
    return ret;
}

float Execution::stackPopFloat(void) {
    return *(float *)&stack[sp--];
}

double Execution::stackPopDouble(void) {
    uint64_t ret;
    sp -= 2;
    ((uint32_t *)&ret)[0] = stack[sp + 1];
    ((uint32_t *)&ret)[1] = stack[sp + 2];
    return *(double *)&ret;
}

MjvmObject *Execution::stackPopObject(void) {
    return (MjvmObject *)stack[sp--];
}

void Execution::stackRestoreContext(void) {
    sp = startSp;
    startSp = stackPopInt32();
    lr = stackPopInt32();
    pc = stackPopInt32();
    method = (const MethodInfo *)stackPopInt32();
    code = method->getAttributeCode().code;
    locals = &stack[startSp + 1];
}

void Execution::initNewContext(const MethodInfo &methodInfo, uint16_t argc) {
    const AttributeCode &attributeCode = methodInfo.getAttributeCode();
    if((sp + attributeCode.maxLocals + attributeCode.maxStack) >= stackLength)
        throw "stack overflow";
    method = &methodInfo;
    code = attributeCode.code;
    pc = 0;
    locals = &stack[sp + 1];
    for(uint32_t i = argc; i < attributeCode.maxLocals; i++) {
        uint32_t index = sp + i + 1;
        stack[index] = 0;
        stackType[index / 8] &= ~(1 << (index % 8));
    }
    sp += attributeCode.maxLocals;
}

const MethodInfo &Execution::findMethod(const ConstMethod &constMethod) {
    const ClassLoader *loader;
    if(constMethod.className == method->classLoader.getThisClass())
        loader = (const ClassLoader *)&method->classLoader;
    else
        loader = &load(constMethod.className);
    while(loader) {
        const MethodInfo *methodInfo = (const MethodInfo *)&loader->getMethodInfo(constMethod.nameAndType);
        if(methodInfo)
            return *methodInfo;
        else
            loader = (const ClassLoader *)&load(loader->getSupperClass());
    }
    throw "can't find the method";
}

void Execution::invoke(const MethodInfo &methodInfo, uint8_t argc) {
    peakSp = sp + 4;
    for(uint32_t i = 0; i < argc; i++)
        setStackValue(sp - i + 4, getStackValue(sp - i));
    sp -= argc;

    /* Save current context */
    stack[++sp] = (int32_t)method;
    stackType[sp / 8] &= ~(1 << (sp % 8));
    stack[++sp] = pc;
    stackType[sp / 8] &= ~(1 << (sp % 8));
    stack[++sp] = lr;
    stackType[sp / 8] &= ~(1 << (sp % 8));
    stack[++sp] = startSp;
    stackType[sp / 8] &= ~(1 << (sp % 8));
    startSp = sp;

    initNewContext(methodInfo, argc);
}

void Execution::invokeStatic(const ConstMethod &constMethod) {
    uint8_t argc = constMethod.parseParamInfo().argc;
    const MethodInfo &methodInfo = findMethod(constMethod);
    if((methodInfo.accessFlag & METHOD_STATIC) == METHOD_STATIC)
        invoke(methodInfo, argc);
    else
        throw "invoke static to non-static method";
}

void Execution::invokeSpecial(const ConstMethod &constMethod) {
    uint8_t argc = constMethod.parseParamInfo().argc + 1;
    const MethodInfo &methodInfo = findMethod(constMethod);
    if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC)
        invoke(methodInfo, argc);
    else
        throw "invoke special to static method";
}

void Execution::invokeVirtual(const ConstMethod &constMethod) {
    uint8_t argc = constMethod.parseParamInfo().argc;
    MjvmObject *obj = (MjvmObject *)stack[sp - argc];
    uint32_t virtualConstMethod[] = {
        (uint32_t)&obj->type,                           /* class name */
        (uint32_t)&constMethod.nameAndType              /* name and type */
    };
    const MethodInfo &methodInfo = findMethod(*(const ConstMethod *)virtualConstMethod);
    if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC) {
        argc++;
        invoke(methodInfo, argc);
    }
    else
        throw "invoke virtual to static method";
}

void Execution::invokeInterface(const ConstInterfaceMethod &interfaceMethod, uint8_t argc) {
    MjvmObject *obj = (MjvmObject *)stack[sp - argc];
    uint32_t interfaceConstMethod[] = {
        (uint32_t)&obj->type,                           /* class name */
        (uint32_t)&interfaceMethod.nameAndType          /* name and type */
    };
    const MethodInfo &methodInfo = findMethod(*(const ConstMethod *)interfaceConstMethod);
    if((methodInfo.accessFlag & METHOD_STATIC) != METHOD_STATIC) {
        argc++;
        invoke(methodInfo, argc);
    }
    else
        throw "invoke interface to static method";
}

bool Execution::isInstanceof(MjvmObject *obj, const ConstUtf8 &type) {
    const char *text = type.getText();
    while(*text == '[')
        text++;
    uint32_t dimensions = text - type.getText();
    uint32_t length = type.length - dimensions;
    if(*text == 'L') {
        text++;
        length -= 2;
    }
    if(length == 16 && obj->dimensions >= dimensions && strncmp(text, "java/lang/Object", length) == 0)
        return true;
    else if(dimensions != obj->dimensions)
        return false;
    else {
        const ConstUtf8 *objType = &obj->type;
        while(1) {
            if(length == objType->length && strncmp(objType->getText(), text, length) == 0)
                return true;
            else {
                objType = &load(*objType).getSupperClass();
                if(objType == 0)
                    return false;
            }
        }
    }
}

MjvmObject *Execution::newMultiArray(const ConstUtf8 &typeName, uint8_t dimensions, int32_t *counts) {
    if(dimensions > 1) {
        MjvmObject *array = newObject(counts[0] * sizeof(MjvmObject *), typeName, dimensions);
        for(uint32_t i = 0; i < counts[0]; i++)
            ((MjvmObject **)(array->data))[i] = newMultiArray(typeName, dimensions - 1, &counts[1]);
        return array;
    }
    else {
        uint8_t atype = isPrimType(typeName);
        uint8_t typeSize = atype ? primitiveTypeSize[atype - 4] : sizeof(MjvmObject *);
        MjvmObject *array = newObject(typeSize * counts[0], typeName, 1);
        memset(array->data, 0, array->size);
        return array;
    }
}

int64_t Execution::run(const char *mainClass) {
    static const void *opcodes[256] = {
        &&op_nop, &&op_aconst_null, &&op_iconst_m1, &&op_iconst_0, &&op_iconst_1, &&op_iconst_2, &&op_iconst_3, &&op_iconst_4, &&op_iconst_5,
        &&op_lconst_0, &&op_lconst_1, &&op_fconst_0, &&op_fconst_1, &&op_fconst_2, &&op_dconst_0, &&op_dconst_1, &&op_bipush, &&op_sipush,
        &&op_ldc, &&op_ldc_w, &&op_ldc2_w, &&op_iload, &&op_lload, &&op_fload, &&op_dload, &&op_aload, &&op_iload_0, &&op_iload_1, &&op_iload_2,
        &&op_iload_3, &&op_lload_0, &&op_lload_1, &&op_lload_2, &&op_lload_3, &&op_fload_0, &&op_fload_1, &&op_fload_2, &&op_fload_3,
        &&op_dload_0, &&op_dload_1, &&op_dload_2, &&op_dload_3, &&op_aload_0, &&op_aload_1, &&op_aload_2, &&op_aload_3, &&op_iaload, &&op_laload,
        &&op_faload, &&op_daload, &&op_aaload, &&op_baload, &&op_caload, &&op_saload, &&op_istore, &&op_lstore, &&op_fstore, &&op_dstore,
        &&op_astore, &&op_istore_0, &&op_istore_1, &&op_istore_2, &&op_istore_3, &&op_lstore_0, &&op_lstore_1, &&op_lstore_2, &&op_lstore_3,
        &&op_fstore_0, &&op_fstore_1, &&op_fstore_2, &&op_fstore_3, &&op_dstore_0, &&op_dstore_1, &&op_dstore_2, &&op_dstore_3, &&op_astore_0,
        &&op_astore_1, &&op_astore_2, &&op_astore_3, &&op_iastore, &&op_lastore, &&op_fastore, &&op_dastore, &&op_aastore, &&op_bastore,
        &&op_castore, &&op_sastore, &&op_pop, &&op_pop2, &&op_dup, &&op_dup_x1, &&op_dup_x2, &&op_dup2, &&op_dup2_x1, &&op_dup2_x2, &&op_unknow,
        &&op_iadd, &&op_ladd, &&op_fadd, &&op_dadd, &&op_isub, &&op_lsub, &&op_fsub, &&op_dsub, &&op_imul, &&op_lmul, &&op_fmul, &&op_dmul,
        &&op_idiv, &&op_ldiv, &&op_fdiv, &&op_ddiv, &&op_irem, &&op_lrem, &&op_frem, &&op_drem, &&op_ineg, &&op_lneg, &&op_fneg, &&op_dneg,
        &&op_ishl, &&op_lshl, &&op_ishr, &&op_lshr, &&op_iushr, &&op_lushr, &&op_iand, &&op_land, &&op_ior, &&op_lor, &&op_ixor, &&op_lxor,
        &&op_iinc, &&op_i2l, &&op_i2f, &&op_i2d, &&op_l2i, &&op_l2f, &&op_l2d, &&op_f2i, &&op_f2l, &&op_f2d, &&op_d2i, &&op_d2l, &&op_d2f,
        &&op_i2b, &&op_i2c, &&op_i2s, &&op_lcmp, &&op_fcmpl, &&op_fcmpg, &&op_dcmpl, &&op_dcmpg, &&op_ifeq, &&op_ifne, &&op_iflt, &&op_ifge,
        &&op_ifgt, &&op_ifle, &&op_if_icmpeq, &&op_if_icmpne, &&op_if_icmplt, &&op_if_icmpge, &&op_if_icmpgt, &&op_if_icmple, &&op_if_acmpeq,
        &&op_if_acmpne, &&op_goto, &&op_jsr, &&op_ret, &&op_tableswitch, &&op_lookupswitch, &&op_ireturn, &&op_lreturn, &&op_freturn,
        &&op_dreturn, &&op_areturn, &&op_return, &&op_getstatic, &&op_putstatic, &&op_getfield, &&op_putfield, &&op_invokevirtual,
        &&op_invokespecial, &&op_invokestatic, &&op_invokeinterface, &&op_invokedynamic, &&op_new, &&op_newarray, &&op_anewarray,
        &&op_arraylength, &&op_athrow, &&op_checkcast, &&op_instanceof, &&op_monitorenter, &&op_monitorexit, &&op_wide, &&op_multianewarray,
        &&op_ifnull, &&op_ifnonnull, &&op_goto_w, &&op_jsrw, &&op_breakpoint, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
        &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow, &&op_unknow,
    };

    method = &load(mainClass).getMainMethodInfo();

    initNewContext(*method);

    goto *opcodes[code[pc]];
    op_nop:
        goto *opcodes[code[pc]];
    op_iconst_m1:
        stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_0:
    op_aconst_null:
        stackPushInt32(0);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_1:
        stackPushInt32(1);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_2:
        stackPushInt32(2);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_3:
        stackPushInt32(3);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_4:
        stackPushInt32(4);
        pc++;
        goto *opcodes[code[pc]];
    op_iconst_5:
        stackPushInt32(5);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_0:
        stackPushInt64(0);
        pc++;
        goto *opcodes[code[pc]];
    op_lconst_1:
        stackPushInt64(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_0:
        stackPushFloat(0);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_1:
        stackPushFloat(1);
        pc++;
        goto *opcodes[code[pc]];
    op_fconst_2:
        stackPushFloat(2);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_0:
        stackPushDouble(0);
        pc++;
        goto *opcodes[code[pc]];
    op_dconst_1:
        stackPushDouble(1);
        pc++;
        goto *opcodes[code[pc]];
    op_bipush:
        stackPushInt32(code[pc + 1]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_sipush:
        stackPushInt32(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        goto *opcodes[code[pc]];
    op_ldc: {
        const ConstPool &constPool = method->classLoader.getConstPool(code[pc + 1]);
        pc += 2;
        switch(constPool.tag) {
            case CONST_INTEGER:
                stackPushInt32(method->classLoader.getConstInteger(constPool));
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(method->classLoader.getConstFloat(constPool));
                goto *opcodes[code[pc]];
            case CONST_STRING:
                Mjvm::lock();
                stackPushObject(getConstString(method->classLoader.getConstString(constPool)));
                Mjvm::unlock();
                goto *opcodes[code[pc]];
            case CONST_CLASS:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
        }
    }
    op_ldc_w: {
        uint16_t index = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstPool &constPool = method->classLoader.getConstPool(index);
        pc += 3;
        switch(constPool.tag) {
            case CONST_INTEGER:
                stackPushInt32(method->classLoader.getConstInteger(constPool));
                goto *opcodes[code[pc]];
            case CONST_FLOAT:
                stackPushFloat(method->classLoader.getConstFloat(constPool));
                goto *opcodes[code[pc]];
            case CONST_STRING:
                Mjvm::lock();
                stackPushObject(getConstString(method->classLoader.getConstString(constPool)));
                Mjvm::unlock();
                goto *opcodes[code[pc]];
            case CONST_CLASS:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_TYPE:
                // TODO
                goto *opcodes[code[pc]];
            case CONST_METHOD_HANDLE:
                // TODO
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
        }
    }
    op_ldc2_w: {
        uint16_t index = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstPool &constPool = method->classLoader.getConstPool(index);
        pc += 3;
        switch(constPool.tag) {
            case CONST_LONG:
                stackPushInt64(method->classLoader.getConstLong(constPool));
                goto *opcodes[code[pc]];
            case CONST_DOUBLE:
                stackPushDouble(method->classLoader.getConstDouble(constPool));
                goto *opcodes[code[pc]];
            default:
                throw "unkown the const pool tag";
        }
    }
    op_iload:
    op_fload:
        stackPushInt32(locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_iload_0:
    op_fload_0:
        stackPushInt32(locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_1:
    op_fload_1:
        stackPushInt32(locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_2:
    op_fload_2:
        stackPushInt32(locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_iload_3:
    op_fload_3:
        stackPushInt32(locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload:
    op_dload:
        stackPushInt64(*(int64_t *)&locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_lload_0:
    op_dload_0:
        stackPushInt64(*(int64_t *)&locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_1:
    op_dload_1:
        stackPushInt64(*(int64_t *)&locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_2:
    op_dload_2:
        stackPushInt64(*(int64_t *)&locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_lload_3:
    op_dload_3:
        stackPushInt64(*(int64_t *)&locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload:
        stackPushObject((MjvmObject *)locals[code[pc + 1]]);
        pc += 2;
        goto *opcodes[code[pc]];
    op_aload_0:
        stackPushObject((MjvmObject *)locals[0]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_1:
        stackPushObject((MjvmObject *)locals[1]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_2:
        stackPushObject((MjvmObject *)locals[2]);
        pc++;
        goto *opcodes[code[pc]];
    op_aload_3:
        stackPushObject((MjvmObject *)locals[3]);
        pc++;
        goto *opcodes[code[pc]];
    op_iaload:
    op_faload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt32(((int32_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_laload:
    op_daload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt64(((int64_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_aaload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushObject(((MjvmObject **)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_baload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt32(((int8_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_caload:
    op_saload: {
        uint32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto load_null_array_excp;
        else if(index < 0 || index >= (obj->size / 4)) {
            // TODO
        }
        stackPushInt32(((int16_t *)obj->data)[index]);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore:
    op_fstore: {
        uint32_t index = code[pc + 1];
        locals[index] = stackPopInt32();
        index = &locals[index] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_lstore:
    op_dstore: {
        uint32_t index = code[pc + 1];
        *(uint64_t *)&locals[index] = stackPopInt64();
        index = &locals[index] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_astore: {
        uint32_t index = code[pc + 1];
        locals[index] = stackPopInt32();
        index = &locals[index] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_istore_0:
    op_fstore_0: {
        locals[0] = stackPopInt32();
        uint32_t index = &locals[0] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_1:
    op_fstore_1: {
        locals[1] = stackPopInt32();
        uint32_t index = &locals[1] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_2:
    op_fstore_2: {
        locals[2] = stackPopInt32();
        uint32_t index = &locals[2] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_istore_3:
    op_fstore_3: {
        locals[3] = stackPopInt32();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_0:
    op_dstore_0: {
        *(uint64_t *)&locals[0] = stackPopInt64();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_1:
    op_dstore_1: {
        *(uint64_t *)&locals[1] = stackPopInt64();
        uint32_t index = &locals[1] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_2:
    op_dstore_2: {
        *(uint64_t *)&locals[2] = stackPopInt64();
        uint32_t index = &locals[2] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lstore_3:
    op_dstore_3: {
        *(uint64_t *)&locals[3] = stackPopInt64();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] &= ~(1 << index % 8);
        index++;
        stackType[index / 8] &= ~(1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_0: {
        locals[0] = stackPopInt32();
        uint32_t index = &locals[0] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_1: {
        locals[1] = stackPopInt32();
        uint32_t index = &locals[1] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_2: {
        locals[2] = stackPopInt32();
        uint32_t index = &locals[2] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_astore_3: {
        locals[3] = stackPopInt32();
        uint32_t index = &locals[3] - stack;
        stackType[index / 8] |= (1 << index % 8);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iastore:
    op_fastore:
    op_aastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / 4))) {
            // TODO
        }
        ((int32_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lastore:
    op_dastore: {
        int64_t value = stackPopInt64();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / 8))) {
            // TODO
        }
        ((int64_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_bastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / 4))) {
            // TODO
        }
        ((int8_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_castore:
    op_sastore: {
        int32_t value = stackPopInt32();
        int32_t index = stackPopInt32();
        MjvmObject *obj = stackPopObject();
        if(obj == 0)
            goto store_null_array_excp;
        else if((index < 0) || (index >= (obj->size / 4))) {
            // TODO
        }
        ((int16_t *)obj->data)[index] = value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_pop:
        stackPopInt32();
        pc++;
        goto *opcodes[code[pc]];
    op_pop2:
        stackPopInt64();
        pc++;
        goto *opcodes[code[pc]];
    op_dup:
        stackPush(getStackValue(sp));
        pc++;
        goto *opcodes[code[pc]];
    op_dup_x1: {
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        stackPush(value1);
        setStackValue(sp - 1, value2);
        setStackValue(sp - 2, value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup_x2: {
        StackValue value3 = getStackValue(sp - 2);
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        stackPush(value1);
        setStackValue(sp - 1, value2);
        setStackValue(sp - 2, value3);
        setStackValue(sp - 3, value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2: {
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        stackPush(value2);
        stackPush(value1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x1: {
        StackValue value3 = getStackValue(sp - 2);
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        stackPush(value2);
        stackPush(value1);
        setStackValue(sp - 2, value3);
        setStackValue(sp - 3, value1);
        setStackValue(sp - 4, value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dup2_x2: {
        StackValue value4 = getStackValue(sp - 3);
        StackValue value3 = getStackValue(sp - 2);
        StackValue value2 = getStackValue(sp - 1);
        StackValue value1 = getStackValue(sp - 0);
        stackPush(value2);
        stackPush(value1);
        setStackValue(sp - 2, value3);
        setStackValue(sp - 3, value4);
        setStackValue(sp - 4, value1);
        setStackValue(sp - 5, value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iadd: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ladd: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fadd: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dadd: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 + value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_isub: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lsub: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fsub: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dsub: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 - value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_imul: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lmul: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fmul: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dmul: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushDouble(value1 * value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_idiv: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt32(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ldiv: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt64(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fdiv: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        stackPushFloat(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ddiv: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        stackPushFloat(value1 / value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_irem: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt32(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lrem: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        if(value2 == 0) {
            // TODO
        }
        stackPushInt64(value1 % value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_frem: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        int32_t temp = (int32_t)(value1 / value2);
        stackPushFloat(value1 - (temp * value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_drem: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        int64_t temp = (int64_t)(value1 / value2);
        stackPushDouble(value1 - (temp * value2));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ineg:
        stack[sp] = -stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lneg:
        stackPushInt64(-stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_fneg:
        stackPushFloat(-stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_dneg:
        stackPushDouble(-stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_ishl: {
        int32_t position = stackPopInt32();
        int32_t value = stackPopInt32();
        stackPushInt32(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshl: {
        int32_t position = stackPopInt32();
        int64_t value = stackPopInt64();
        stackPushInt64(value << position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ishr: {
        int32_t position = stackPopInt32();
        int32_t value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lshr: {
        int32_t position = stackPopInt32();
        int64_t value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iushr: {
        int32_t position = stackPopInt32();
        uint32_t value = stackPopInt32();
        stackPushInt32(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lushr: {
        int32_t position = stackPopInt32();
        uint64_t value = stackPopInt64();
        stackPushInt64(value >> position);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iand: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_land: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 & value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ior: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lor: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 | value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ixor: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        stackPushInt32(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_lxor: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt64(value1 ^ value2);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_iinc:
        locals[code[pc + 1]] += (int8_t )code[pc + 2];
        pc += 3;
        goto *opcodes[code[pc]];
    op_i2l:
        stackPushInt64(stackPopInt32());
        pc++;
        goto *opcodes[code[pc]];
    op_i2f: {
        float value = stack[sp];
        stack[sp] = *(int32_t *)&value;
        pc++;
        goto *opcodes[code[pc]];
    }
    op_i2d:
        stackPushDouble(stackPopInt32());
        pc++;
        goto *opcodes[code[pc]];
    op_l2i:
        stackPushInt32(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2f:
        stackPushFloat(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_l2d:
        stackPushDouble(stackPopInt64());
        pc++;
        goto *opcodes[code[pc]];
    op_f2i:
        stack[sp] = *(float *)&stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_f2l:
        stackPushInt64(stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_f2d:
        stackPushDouble(stackPopFloat());
        pc++;
        goto *opcodes[code[pc]];
    op_d2i:
        stackPushInt32(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_d2l:
        stackPushInt64(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_d2f:
        stackPushFloat(stackPopDouble());
        pc++;
        goto *opcodes[code[pc]];
    op_i2b:
        stack[sp] = (int8_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_i2c:
    op_i2s:
        stack[sp] = (int16_t)stack[sp];
        pc++;
        goto *opcodes[code[pc]];
    op_lcmp: {
        int64_t value2 = stackPopInt64();
        int64_t value1 = stackPopInt64();
        stackPushInt32((value1 == value2) ? 0 : ((value1 < value2) ? -1 : 0));
        pc++;
        goto *opcodes[code[pc]];
    }
    op_fcmpl:
    op_fcmpg: {
        float value2 = stackPopFloat();
        float value1 = stackPopFloat();
        if((*(uint32_t *)&value1 == FLOAT_NAN) || (*(uint32_t *)&value2 == FLOAT_NAN))
            stackPushInt32((code[pc] == OP_FCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_dcmpl:
    op_dcmpg: {
        double value2 = stackPopDouble();
        double value1 = stackPopDouble();
        if((*(uint64_t *)&value1 == DOUBLE_NAN) || (*(uint64_t *)&value2 == DOUBLE_NAN))
            stackPushInt32((code[pc] == OP_DCMPL) ? -1 : 1);
        else if(value1 > value2)
            stackPushInt32(1);
        else if(value1 == value2)
            stackPushInt32(0);
        else
            stackPushInt32(-1);
        pc++;
        goto *opcodes[code[pc]];
    }
    op_ifeq:
    op_ifnull:
        pc += (!stackPopInt32()) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifne:
    op_ifnonnull:
        pc += stackPopInt32() ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_iflt:
        pc += (stackPopInt32() < 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifge:
        pc += (stackPopInt32() >= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifgt:
        pc += (stackPopInt32() > 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_ifle:
        pc += (stackPopInt32() <= 0) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    op_if_icmpeq:
    op_if_acmpeq: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 == value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpne:
    op_if_acmpne: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 != value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmplt: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 < value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpge: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 >= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmpgt: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 > value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_if_icmple: {
        int32_t value2 = stackPopInt32();
        int32_t value1 = stackPopInt32();
        pc += (value1 <= value2) ? ARRAY_TO_INT16(&code[pc + 1]) : 3;
        goto *opcodes[code[pc]];
    }
    op_goto:
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_goto_w:
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsr:
        stackPushInt32(pc + 3);
        pc += ARRAY_TO_INT16(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_jsrw:
        stackPushInt32(pc + 5);
        pc += ARRAY_TO_INT32(&code[pc + 1]);
        goto *opcodes[code[pc]];
    op_ret:
        pc = locals[code[pc + 1]];
        goto *opcodes[code[pc]];
    op_tableswitch: {
        int32_t index = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t low = ARRAY_TO_INT32(&table[4]);
        int32_t height = ARRAY_TO_INT32(&table[8]);
        if(index < low || index > height) {
            int32_t defaultOffset = ARRAY_TO_INT32(table);
            pc += defaultOffset;
            goto *opcodes[code[pc]];
        }
        table = &table[12 + (index - low) * 4];
        pc += ARRAY_TO_INT32(table);
        goto *opcodes[code[pc]];
    }
    op_lookupswitch: {
        int32_t key = stackPopInt32();
        uint8_t padding = (4 - ((pc + 1) % 4)) % 4;
        const uint8_t *table = &code[pc + padding + 1];
        int32_t defaultPc = ARRAY_TO_INT32(table);
        int32_t npairs = ARRAY_TO_INT32(&table[4]);
        table = &table[8];
        while(npairs--) {
            int32_t pairs = ARRAY_TO_INT32(table);
            if(key == pairs) {
                pc += ARRAY_TO_INT32(&table[4]);
                goto *opcodes[code[pc]];
            }
            table = &table[8];
        }
        pc += defaultPc;
        goto *opcodes[code[pc]];
    }
    op_ireturn:
    op_freturn: {
        int32_t retVal = stackPopInt32();
        if(startSp < 0) {
            sp = peakSp = startSp;
            return retVal;
        }
        stackRestoreContext();
        stackPushInt32(retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_lreturn:
    op_dreturn: {
        int64_t retVal = stackPopInt64();
        if(startSp < 0) {
            sp = peakSp = startSp;
            return retVal;
        }
        stackRestoreContext();
        stackPushInt64(retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_areturn: {
        int32_t retVal = (int32_t)stackPopObject();
        if(startSp < 0) {
            sp = peakSp = startSp;
            return retVal;
        }
        stackRestoreContext();
        stackPushObject((MjvmObject *)retVal);
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_return: {
        if(startSp < 0) {
            sp = peakSp = startSp;
            return 0;
        }
        stackRestoreContext();
        peakSp = sp;
        pc = lr;
        goto *opcodes[code[pc]];
    }
    op_getstatic: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        const FieldsData &fields = getStaticFields(constField.className);
        if((int32_t)&fields == 0) {
            stackPushInt32((int32_t)&load(constField.className));
            goto init_static_field;
        }
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                stackPushInt32(fields.getFieldData8(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                stackPushInt32(fields.getFieldData16(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                stackPushInt64(fields.getFieldData64(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                stackPushObject(fields.getFieldObject(constField.nameAndType).object);
                pc += 3;
                goto *opcodes[code[pc]];
            }
            default: {
                stackPushInt32(fields.getFieldData32(constField.nameAndType).value);
                pc += 3;
                goto *opcodes[code[pc]];
            }
        }
    }
    op_putstatic: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        const FieldsData &fields = getStaticFields(constField.className);
        if((int32_t)&fields == 0) {
            stackPushInt32((int32_t)&load(constField.className));
            goto init_static_field;
        }
        pc += 3;
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                fields.getFieldData8(constField.nameAndType).value = stackPopInt32();
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                fields.getFieldData16(constField.nameAndType).value = stackPopInt32();
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                fields.getFieldData64(constField.nameAndType).value = stackPopInt64();
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                fields.getFieldObject(constField.nameAndType).object = stackPopObject();
                goto *opcodes[code[pc]];
            }
            default: {
                fields.getFieldData32(constField.nameAndType).value = stackPopInt32();
                goto *opcodes[code[pc]];
            }
        }
    }
    op_getfield: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt32(fields.getFieldData8(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt32(fields.getFieldData16(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt64(fields.getFieldData64(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushObject(fields.getFieldObject(constField.nameAndType).object);
                goto *opcodes[code[pc]];
            }
            default: {
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto getfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                stackPushInt32(fields.getFieldData32(constField.nameAndType).value);
                goto *opcodes[code[pc]];
            }
        }
        getfield_null_excp: {
            const char *msg[] = {"Cannot read field ", constField.nameAndType.name.getText(), " from null object"};
            Mjvm::lock();
            MjvmObject *strObj = newString(msg, LENGTH(msg));
            MjvmObject *excpObj = newNullPointerException(strObj);
            stackPushObject(excpObj);
            Mjvm::unlock();
            goto exception_handler;
        }
    }
    op_putfield: {
        const ConstField &constField = method->classLoader.getConstField(ARRAY_TO_INT16(&code[pc + 1]));
        pc += 3;
        switch(constField.nameAndType.descriptor.getText()[0]) {
            case 'Z':
            case 'B': {
                int32_t value = stackPopInt32();
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData8(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
            case 'C':
            case 'S': {
                int32_t value = stackPopInt32();
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData16(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
            case 'J':
            case 'D': {
                int64_t value = stackPopInt64();
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData64(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
            case 'L':
            case '[': {
                MjvmObject *value = stackPopObject();
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldObject(constField.nameAndType).object = value;
                goto *opcodes[code[pc]];
            }
            default: {
                int32_t value = stackPopInt32();
                MjvmObject *obj = stackPopObject();
                if(obj == 0)
                    goto putfield_null_excp;
                const FieldsData &fields = *(FieldsData *)obj->data;
                fields.getFieldData32(constField.nameAndType).value = value;
                goto *opcodes[code[pc]];
            }
        }
        putfield_null_excp: {
            const char *msg[] = {"Cannot assign field ", constField.nameAndType.name.getText(), " for null object"};
            Mjvm::lock();
            MjvmObject *strObj = newString(msg, LENGTH(msg));
            MjvmObject *excpObj = newNullPointerException(strObj);
            stackPushObject(excpObj);
            Mjvm::unlock();
            goto exception_handler;
        }
    }
    op_invokevirtual: {
        const ConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        lr = pc + 3;
        invokeVirtual(constMethod);
        goto *opcodes[code[pc]];
    }
    op_invokespecial: {
        const ConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        lr = pc + 3;
        invokeSpecial(constMethod);
        goto *opcodes[code[pc]];
    }
    op_invokestatic: {
        const ConstMethod &constMethod = method->classLoader.getConstMethod(ARRAY_TO_INT16(&code[pc + 1]));
        lr = pc + 3;
        invokeStatic(constMethod);
        goto *opcodes[code[pc]];
    }
    op_invokeinterface: {
        const ConstInterfaceMethod &interfaceMethod = method->classLoader.getConstInterfaceMethod(ARRAY_TO_INT16(&code[pc + 1]));
        uint8_t count = code[pc + 3];
        lr = pc + 5;
        invokeInterface(interfaceMethod, count);
        goto *opcodes[code[pc]];
    }
    op_invokedynamic:
        // TODO
        goto *opcodes[code[pc]];
    op_new: {
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstUtf8 &constClass =  method->classLoader.getConstClass(poolIndex);
        Mjvm::lock();
        MjvmObject *obj = newObject(sizeof(FieldsData), constClass);
        new ((FieldsData *)obj->data)FieldsData(*this, load(constClass), false);
        stackPushObject(obj);
        Mjvm::unlock();
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_newarray: {
        int32_t count = stackPopInt32();
        if(count <= 0) {
            // TODO
        }
        uint8_t atype = code[pc + 1];
        uint8_t typeSize = primitiveTypeSize[atype - 4];
        Mjvm::lock();
        MjvmObject *obj = newObject(typeSize * count, *primTypeConstUtf8List[atype - 4], 1);
        stackPushObject(obj);
        Mjvm::unlock();
        memset(obj->data, 0, obj->size);
        pc += 2;
        goto *opcodes[code[pc]];
    }
    op_anewarray: {
        int32_t count = stackPopInt32();
        uint16_t poolIndex = ARRAY_TO_INT16(&code[pc + 1]);
        const ConstUtf8 &constClass =  method->classLoader.getConstClass(poolIndex);
        Mjvm::lock();
        MjvmObject *obj = newObject(4 * count, constClass, 1);
        stackPushObject(obj);
        Mjvm::unlock();
        memset(obj->data, 0, obj->size);
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_arraylength: {
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            Mjvm::lock();
            MjvmObject *strObj = newString("Cannot read the array length from null object");
            MjvmObject *excpObj = newNullPointerException(strObj);
            stackPushObject(excpObj);
            Mjvm::unlock();
            goto exception_handler;
        }
        stackPushInt32(obj->size / obj->parseTypeSize());
        pc++;
        goto *opcodes[code[pc]];
    }
    op_athrow: {
        MjvmObject *obj = (MjvmObject *)stack[sp];
        if(obj == 0) {
            stackPopObject();
            // TODO
        }
        goto exception_handler;
    }
    exception_handler: {
        MjvmObject *obj = stackPopObject();
        sp = startSp + method->getAttributeCode().maxLocals;
        stackPushObject(obj);
        const AttributeCode &attributeCode = method->getAttributeCode();
        for(uint16_t i = 0; i < attributeCode.exceptionTableLength; i++) {
            const ExceptionTable &exceptionTable = attributeCode.getException(i);
            if(exceptionTable.startPc <= pc && pc < exceptionTable.endPc) {
                if(isInstanceof(obj, method->classLoader.getConstClass(exceptionTable.catchType))) {
                    pc = exceptionTable.handlerPc;
                    goto *opcodes[code[pc]];
                }
            }
        }
        if(startSp < 0)
            throw (MjvmString *)obj;
        stackRestoreContext();
        sp = startSp + method->getAttributeCode().maxLocals;
        stackPushObject(obj);
        goto exception_handler;
    }
    op_checkcast: {
        MjvmObject *obj = (MjvmObject *)stack[sp];
        const ConstUtf8 &type = method->classLoader.getConstClass(ARRAY_TO_INT16(&code[pc + 1]));
        if(obj != 0 && !isInstanceof(obj, type)) {
            const char *msg[] = {"Class ", obj->type.getText(), " cannot be cast to class ", type.getText()};
            Mjvm::lock();
            MjvmObject *strObj = newString(msg, LENGTH(msg));
            MjvmObject *excpObj = newNullPointerException(strObj);
            stackPushObject(excpObj);
            Mjvm::unlock();
            goto exception_handler;
        }
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_instanceof: {
        MjvmObject *obj = stackPopObject();
        const ConstUtf8 &type = method->classLoader.getConstClass(ARRAY_TO_INT16(&code[pc + 1]));
        stackPushInt32(isInstanceof(obj, type));
        pc += 3;
        goto *opcodes[code[pc]];
    }
    op_monitorenter: {
        MjvmObject *obj = stackPopObject();
        if(obj == 0) {
            Mjvm::lock();
            MjvmObject *strObj = newString("Cannot enter synchronized block by null object");
            MjvmObject *excpObj = newNullPointerException(strObj);
            stackPushObject(excpObj);
            Mjvm::unlock();
            goto exception_handler;
        }
        Mjvm::lock();
        if(obj->monitorCount == 0 || obj->ownId == (int32_t)this) {
            obj->ownId = (int32_t)this;
            if(obj->monitorCount < 0xFFFFFF)
                obj->monitorCount++;
            else {
                // TODO
                Mjvm::unlock();
                goto exception_handler;
            }
            pc++;
        }
        Mjvm::unlock();
        goto *opcodes[code[pc]];
    }
    op_monitorexit: {
        MjvmObject *obj = stackPopObject();
        Mjvm::lock();
        obj->monitorCount--;
        Mjvm::unlock();
        pc++;
        goto *opcodes[code[pc]];
    }
    op_wide:
        // TODO
        goto *opcodes[code[pc]];
    op_multianewarray: {
        const ConstUtf8 *typeName = &method->classLoader.getConstClass(ARRAY_TO_INT16(&code[pc + 1]));
        const uint8_t dimensions = code[pc + 3];
        const char *typeNameText = typeName->getText();
        uint32_t length = typeName->length - dimensions;
        if(typeNameText[dimensions] != 'L') {
            uint8_t atype = convertToAType(typeNameText[dimensions]);
            if(atype == 0)
                throw "invalid primative type";
            typeName = primTypeConstUtf8List[atype - 4];
        }
        else
            typeName = &load(&typeNameText[dimensions + 1], length - 2).getThisClass();
        sp -= dimensions - 1;
        Mjvm::lock();
        MjvmObject *array = newMultiArray(*typeName, dimensions, &stack[sp]);
        stackPushObject(array);
        Mjvm::unlock();
        pc += 4;
        goto *opcodes[code[pc]];
    }
    op_breakpoint:
        pc++;
        goto *opcodes[code[pc]];
    op_unknow:
        throw "unknow opcode";
    init_static_field: {
        const ClassLoader &classToInit = *(const ClassLoader *)stackPopInt32();
        static const uint32_t nameAndType[] = {
            (uint32_t)"\x08\x00<clinit>",               /* method name */
            (uint32_t)"\x03\x00()V"                     /* method type */
        };
        uint32_t ctorConstMethod[] = {
            (uint32_t)&classToInit.getThisClass(),      /* class name */
            (uint32_t)nameAndType                       /* method type */
        };
        initStaticField(classToInit);
        lr = pc;
        invokeStatic(*(const ConstMethod *)ctorConstMethod);
        goto *opcodes[code[pc]];
    }
    load_null_array_excp: {
        Mjvm::lock();
        MjvmObject *strObj = newString("Cannot load from null array object");
        MjvmObject *excpObj = newNullPointerException(strObj);
        stackPushObject(excpObj);
        Mjvm::unlock();
        goto exception_handler;
    }
    store_null_array_excp: {
        Mjvm::lock();
        MjvmObject *strObj = newString("Cannot store to null array object");
        MjvmObject *excpObj = newNullPointerException(strObj);
        stackPushObject(excpObj);
        Mjvm::unlock();
        goto exception_handler;
    }
}

Execution::~Execution(void) {
    Mjvm::free(stack);
    Mjvm::free(stackType);
    if(staticClassDataList) {
        for(ClassDataNode *node = staticClassDataList; node != 0;) {
            ClassDataNode *next = node->next;
            Mjvm::destroy(node->classLoader);
            node->~ClassDataNode();
            Mjvm::free(node);
            node = next;
        }
    }
    freeAllObject();
}
