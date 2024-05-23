
#include <iostream>
#include <string.h>
#include "mjvm.h"
#include "mjvm_system_api.h"
#include "mjvm_class_loader.h"

static void *ClassLoader_Open(const char *fileName) {
    char buff[256];
    uint32_t i = 0;
    while(fileName[i]) {
        buff[i] = fileName[i];
        i++;
    }
    buff[i++] = '.';
    buff[i++] = 'c';
    buff[i++] = 'l';
    buff[i++] = 'a';
    buff[i++] = 's';
    buff[i++] = 's';
    buff[i++] = 0;

    void *file = MjvmSystem_FileOpen(buff, MJVM_FILE_READ);
    if(file == 0)
        throw "can not open file";
    return file;
}

static void *ClassLoader_Open(const char *fileName, uint32_t length) {
    char buff[256];
    uint32_t i = 0;
    while(i < length) {
        buff[i] = fileName[i];
        i++;
    }
    buff[i++] = '.';
    buff[i++] = 'c';
    buff[i++] = 'l';
    buff[i++] = 'a';
    buff[i++] = 's';
    buff[i++] = 's';
    buff[i++] = 0;

    void *file = MjvmSystem_FileOpen(buff, MJVM_FILE_READ);
    if(file == 0)
        throw "can not open file";
    return file;
}

static void ClassLoader_Read(void *file, void *buff, uint32_t size) {
    uint32_t temp;
    MjvmSys_FileResult ret = MjvmSystem_FileRead(file, buff, size, &temp);
    if((ret != FILE_RESULT_OK) || (temp != size))
        throw "read file error";
}

static uint8_t ClassLoader_ReadUInt8(void *file) {
    uint8_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return temp;
}

static uint16_t ClassLoader_ReadUInt16(void *file) {
    uint16_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return swap16(temp);
}

static uint32_t ClassLoader_ReadUInt32(void *file) {
    uint32_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return swap32(temp);
}

static uint64_t ClassLoader_ReadUInt64(void *file) {
    uint64_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return swap64(temp);
}

static void ClassLoader_Seek(void *file, int32_t offset) {
    if(MjvmSystem_FileSeek(file, MjvmSystem_FileTell(file) + offset) != FILE_RESULT_OK)
        throw "read file error";
}

ClassLoader::ClassLoader(const char *fileName) {
    poolCount = 0;
    interfacesCount = 0;
    fieldsCount = 0;
    methodsCount = 0;
    attributesCount = 0;

    void *file = ClassLoader_Open(fileName);

    try {
        readFile(file);
    }
    catch(const char *excp) {
        MjvmSystem_FileClose(file);
        throw excp;
    }

    MjvmSystem_FileClose(file);
}

ClassLoader::ClassLoader(const char *fileName, uint16_t length) {
    poolCount = 0;
    interfacesCount = 0;
    fieldsCount = 0;
    methodsCount = 0;
    attributesCount = 0;

    void *file = ClassLoader_Open(fileName, length);

    try {
        readFile(file);
    }
    catch(const char *excp) {
        MjvmSystem_FileClose(file);
        throw excp;
    }

    MjvmSystem_FileClose(file);
}

ClassLoader::ClassLoader(const ConstUtf8 &fileName) : ClassLoader(fileName.text, fileName.length) {

}

void ClassLoader::readFile(void *file) {
    magic = ClassLoader_ReadUInt32(file);
    minorVersion = ClassLoader_ReadUInt16(file);
    majorVersion = ClassLoader_ReadUInt16(file);
    poolCount = ClassLoader_ReadUInt16(file) - 1;
    poolTable = (ConstPool *)Mjvm::malloc(poolCount * sizeof(ConstPool));
    for(uint32_t i = 0; i < poolCount; i++) {
        *(ConstPoolTag *)&poolTable[i].tag = (ConstPoolTag)ClassLoader_ReadUInt8(file);
        switch(poolTable[i].tag) {
            case CONST_UTF8: {
                uint16_t length = ClassLoader_ReadUInt16(file);
                *(uint32_t *)&poolTable[i].value = (uint32_t)Mjvm::malloc(sizeof(ConstUtf8) + length + 1);
                *(uint16_t *)&((ConstUtf8 *)poolTable[i].value)->length = length;
                char *textBuff = (char *)((ConstUtf8 *)poolTable[i].value)->text;
                ClassLoader_Read(file, textBuff, length);
                textBuff[length] = 0;
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
                *(uint32_t *)&poolTable[i].value = ClassLoader_ReadUInt32(file);
                break;
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                ((uint16_t *)&poolTable[i].value)[0] = ClassLoader_ReadUInt16(file);
                ((uint16_t *)&poolTable[i].value)[1] = ClassLoader_ReadUInt16(file);
                break;
            case CONST_LONG:
            case CONST_DOUBLE: {
                uint64_t value = ClassLoader_ReadUInt64(file);
                *(uint32_t *)&poolTable[i + 0].value = (uint32_t)value;
                *(uint32_t *)&poolTable[i + 1].value = (uint32_t)(value >> 32);
                *(ConstPoolTag *)&poolTable[i + 1].tag = CONST_UNKOWN;
                i++;
                break;
            }
            case CONST_STRING:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
            case CONST_CLASS:
            case CONST_METHOD_TYPE:
                *(uint32_t *)&poolTable[i].value = ClassLoader_ReadUInt16(file);
                break;
            case CONST_METHOD_HANDLE:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                ((uint8_t *)&poolTable[i].value)[0] = ClassLoader_ReadUInt8(file);
                ((uint16_t *)&poolTable[i].value)[1] = ClassLoader_ReadUInt16(file);
                break;
            default:
                throw "uknow pool type";
        }
    }
    accessFlags = ClassLoader_ReadUInt16(file);
    thisClass = ClassLoader_ReadUInt16(file);
    superClass = ClassLoader_ReadUInt16(file);
    interfacesCount = ClassLoader_ReadUInt16(file);
    if(interfacesCount) {
        interfaces = (uint16_t *)Mjvm::malloc(interfacesCount * sizeof(uint16_t));
        ClassLoader_Read(file, interfaces, interfacesCount * sizeof(uint16_t));
    }
    fieldsCount = ClassLoader_ReadUInt16(file);
    if(fieldsCount) {
        uint32_t count = 0;
        fields = (FieldInfo *)Mjvm::malloc(fieldsCount * sizeof(FieldInfo));
        for(uint16_t i = 0; i < fieldsCount; i++) {
            FieldAccessFlag flag = (FieldAccessFlag)ClassLoader_ReadUInt16(file);
            uint16_t fieldsNameIndex = ClassLoader_ReadUInt16(file);
            uint16_t fieldsDescriptorIndex = ClassLoader_ReadUInt16(file);
            uint16_t fieldsAttributesCount = ClassLoader_ReadUInt16(file);
            while(fieldsAttributesCount--) {
                uint16_t nameIndex = ClassLoader_ReadUInt16(file);
                uint32_t length = ClassLoader_ReadUInt32(file);
                AttributeType type = AttributeInfo::parseAttributeType(getConstUtf8(nameIndex));
                if(type == ATTRIBUTE_CONSTANT_VALUE)
                    flag = (FieldAccessFlag)(flag | FIELD_UNLOAD);
                ClassLoader_Seek(file, length);
            }
            if(!(flag & FIELD_UNLOAD)) {
                new (&fields[count])FieldInfo(*this, flag, getConstUtf8(fieldsNameIndex), getConstUtf8(fieldsDescriptorIndex));
                count++;
            }
        }
        if(count == 0) {
            Mjvm::free(fields);
            fields = 0;
        }
        else if(count != fieldsCount) {
            fields = (FieldInfo *)Mjvm::realloc(fields, count * sizeof(FieldInfo));
            fieldsCount = count;
        }
    }
    methodsCount = ClassLoader_ReadUInt16(file);
    if(methodsCount) {
        methods = (MethodInfo *)Mjvm::malloc(methodsCount * sizeof(MethodInfo));
        for(uint16_t i = 0; i < methodsCount; i++) {
            MethodAccessFlag flag = (MethodAccessFlag)ClassLoader_ReadUInt16(file);
            uint16_t methodNameIndex = ClassLoader_ReadUInt16(file);
            uint16_t methodDescriptorIndex = ClassLoader_ReadUInt16(file);
            uint16_t methodAttributesCount = ClassLoader_ReadUInt16(file);
            new (&methods[i])MethodInfo(*this, flag, getConstUtf8(methodNameIndex), getConstUtf8(methodDescriptorIndex));
            if((flag & METHOD_NATIVE) == METHOD_NATIVE) {
                AttributeNative *attrNative = (AttributeNative *)Mjvm::malloc(sizeof(AttributeNative));
                new (attrNative)AttributeNative(0);
                methods[i].addAttribute(attrNative);
            }
            while(methodAttributesCount--) {
                AttributeInfo *attr = readAttribute(file);
                if(attr != 0)
                    methods[i].addAttribute(attr);
            }
        }
    }
    attributesCount = ClassLoader_ReadUInt16(file);
    while(attributesCount--) {
        AttributeInfo *attr = readAttribute(file);
        if(attr)
            addAttribute(attr);
    }
}

void ClassLoader::addAttribute(AttributeInfo *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

AttributeInfo *ClassLoader::readAttribute(void *file) {
    uint16_t nameIndex = ClassLoader_ReadUInt16(file);
    uint32_t length = ClassLoader_ReadUInt32(file);
    AttributeType type = AttributeInfo::parseAttributeType(getConstUtf8(nameIndex));
    switch(type) {
        case ATTRIBUTE_CODE:
            return readAttributeCode(file);
        case ATTRIBUTE_BOOTSTRAP_METHODS:
            return readAttributeBootstrapMethods(file);
        default:
            ClassLoader_Seek(file, length);
            return 0;
    }
}

AttributeInfo *ClassLoader::readAttributeCode(void *file) {
    AttributeCode *attribute = (AttributeCode *)Mjvm::malloc(sizeof(AttributeCode));
    uint16_t maxStack = ClassLoader_ReadUInt16(file);
    uint16_t maxLocals = ClassLoader_ReadUInt16(file);
    uint32_t codeLength = ClassLoader_ReadUInt32(file);
    uint8_t *code = (uint8_t *)Mjvm::malloc(codeLength);
    new (attribute)AttributeCode(maxStack, maxLocals);
    ClassLoader_Read(file, code, codeLength);
    attribute->setCode(code, codeLength);
    uint16_t exceptionTableLength = ClassLoader_ReadUInt16(file);
    if(exceptionTableLength) {
        ExceptionTable *exceptionTable = (ExceptionTable *)Mjvm::malloc(exceptionTableLength * sizeof(ExceptionTable));
        attribute->setExceptionTable(exceptionTable, exceptionTableLength);
        for(uint16_t i = 0; i < exceptionTableLength; i++) {
            uint16_t startPc = ClassLoader_ReadUInt16(file);
            uint16_t endPc = ClassLoader_ReadUInt16(file);
            uint16_t handlerPc = ClassLoader_ReadUInt16(file);
            uint16_t catchType = ClassLoader_ReadUInt16(file);
            new (&exceptionTable[i])ExceptionTable(startPc, endPc, handlerPc, catchType);
        }
    }
    uint16_t attrbutesCount = ClassLoader_ReadUInt16(file);
    while(attrbutesCount--) {
        AttributeInfo *attr = readAttribute(file);
        if(attr)
            attribute->addAttribute(attr);
    }
    return attribute;
}

AttributeInfo *ClassLoader::readAttributeBootstrapMethods(void *file) {
    uint16_t numBootstrapMethods = ClassLoader_ReadUInt16(file);
    AttributeBootstrapMethods *attribute = (AttributeBootstrapMethods *)Mjvm::malloc(sizeof(AttributeBootstrapMethods));
    new (attribute)AttributeBootstrapMethods(numBootstrapMethods);
    for(uint16_t i = 0; i < numBootstrapMethods; i++) {
        uint16_t bootstrapMethodRef = ClassLoader_ReadUInt16(file);
        uint16_t numBootstrapArguments = ClassLoader_ReadUInt16(file);
        BootstrapMethod *bootstrapMethod = (BootstrapMethod *)Mjvm::malloc(sizeof(BootstrapMethod) + numBootstrapArguments * sizeof(uint16_t));
        new (bootstrapMethod)BootstrapMethod(bootstrapMethodRef, numBootstrapArguments);
        uint16_t *bootstrapArguments = (uint16_t *)(((uint8_t *)bootstrapMethod) + sizeof(BootstrapMethod));
        ClassLoader_Read(file, bootstrapArguments, numBootstrapArguments * sizeof(uint16_t));
        attribute->setBootstrapMethod(i, *bootstrapMethod);
    }
    return attribute;
}

uint32_t ClassLoader::getMagic(void) const {
    return magic;
}

uint16_t ClassLoader::getMinorVersion(void) const {
    return minorVersion;
}

uint16_t ClassLoader::getMajorversion(void) const {
    return majorVersion;
}

const ConstPool &ClassLoader::getConstPool(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount)
        return poolTable[poolIndex];
    throw "index for const pool is invalid";
}

int32_t ClassLoader::getConstInteger(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_INTEGER)
        return (int32_t)poolTable[poolIndex].value;
    throw "index for const integer is invalid";
}

int32_t ClassLoader::getConstInteger(const ConstPool &constPool) const {
    if(constPool.tag == CONST_INTEGER)
        return (int32_t)constPool.value;
    throw "const pool tag is not integer tag";
}

float ClassLoader::getConstFloat(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_FLOAT)
        return *(float *)&poolTable[poolIndex].value;
    throw "index for const float is invalid";
}

float ClassLoader::getConstFloat(const ConstPool &constPool) const {
    if(constPool.tag == CONST_FLOAT)
        return *(float *)&constPool.value;
    throw "const pool tag is not float tag";
}

int64_t ClassLoader::getConstLong(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_LONG)
        return ((uint64_t)poolTable[poolIndex + 1].value << 32) | poolTable[poolIndex].value;
    throw "index for const long is invalid";
}

int64_t ClassLoader::getConstLong(const ConstPool &constPool) const {
    return getConstLong((uint16_t)(&constPool - poolTable) + 1);
}

double ClassLoader::getConstDouble(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_DOUBLE) {
        uint64_t ret = ((uint64_t)poolTable[poolIndex + 1].value << 32) | poolTable[poolIndex].value;
        return *(double *)&ret;
    }
    throw "index for const double is invalid";
}

double ClassLoader::getConstDouble(const ConstPool &constPool) const {
    return getConstDouble((uint16_t)(&constPool - poolTable) + 1);
}

const ConstUtf8 &ClassLoader::getConstUtf8(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_UTF8)
        return *(ConstUtf8 *)poolTable[poolIndex].value;
    throw "index for const utf8 is invalid";
}

const ConstUtf8 &ClassLoader::getConstUtf8(const ConstPool &constPool) const {
    if(constPool.tag == CONST_UTF8)
        return *(ConstUtf8 *)constPool.value;
    throw "const pool tag is not utf8 tag";
}

const ConstUtf8 &ClassLoader::getConstClass(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_CLASS)
        return getConstUtf8(poolTable[poolIndex].value);
    throw "index for const class is invalid";
}

const ConstUtf8 &ClassLoader::getConstClass(const ConstPool &constPool) const {
    if(constPool.tag == CONST_CLASS)
        return getConstUtf8(constPool.value);
    throw "const pool tag is not class tag";
}

MjvmString &ClassLoader::getConstString(Execution &execution, uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_STRING) {
        if(poolTable[poolIndex].tag & 0x80) {
            const ConstUtf8 &utf8Str = getConstUtf8(poolTable[poolIndex].value);
            MjvmString *strObj = execution.getConstString(utf8Str);
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)strObj;
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_STRING;
        }
        return *(MjvmString *)poolTable[poolIndex].value;
    }
    throw "index for const string is invalid";
}

MjvmString &ClassLoader::getConstString(Execution &execution, const ConstPool &constPool) const {
    if((constPool.tag & 0x7F) == CONST_STRING) {
        if(constPool.tag & 0x80) {
            const ConstUtf8 &utf8Str = getConstUtf8(constPool.value);
            MjvmString *strObj = execution.getConstString(utf8Str);
            *(uint32_t *)&constPool.value = (uint32_t)strObj;
            *(ConstPoolTag *)&constPool.tag = CONST_STRING;
        }
        return *(MjvmString *)constPool.value;
    }
    throw "const pool tag is not string tag";
}

const ConstUtf8 &ClassLoader::getConstMethodType(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_METHOD)
        return getConstUtf8(poolTable[poolIndex].value);
    throw "index for const method type is invalid";
}

const ConstUtf8 &ClassLoader::getConstMethodType(const ConstPool &constPool) const {
    if(constPool.tag == CONST_METHOD)
        return getConstUtf8(constPool.value);
    throw "const pool tag is not method type tag";
}

const ConstNameAndType &ClassLoader::getConstNameAndType(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_NAME_AND_TYPE) {
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t nameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t descriptorIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_NAME_AND_TYPE;
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(ConstNameAndType));
            new ((ConstNameAndType *)poolTable[poolIndex].value)ConstNameAndType(getConstUtf8(nameIndex), getConstUtf8(descriptorIndex));
        }
        return *(ConstNameAndType *)poolTable[poolIndex].value;
    }
    throw "index for const name and type is invalid";
}

const ConstNameAndType &ClassLoader::getConstNameAndType(const ConstPool &constPool) const {
    return getConstNameAndType((uint16_t)(&constPool - poolTable) + 1);
}

const ConstField &ClassLoader::getConstField(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_FIELD) {
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_FIELD;
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(ConstField));
            new ((ConstField *)poolTable[poolIndex].value)ConstField(getConstClass(classNameIndex), getConstNameAndType(nameAndTypeIndex));
        }
        return *(ConstField *)poolTable[poolIndex].value;
    }
    throw "index for const field is invalid";
}

const ConstField &ClassLoader::getConstField(const ConstPool &constPool) const {
    return getConstField((uint16_t)(&constPool - poolTable) + 1);
}

const ConstMethod &ClassLoader::getConstMethod(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_METHOD) {
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_METHOD;
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(ConstMethod));
            new ((ConstMethod *)poolTable[poolIndex].value)ConstMethod(getConstClass(classNameIndex), getConstNameAndType(nameAndTypeIndex));
        }
        return *(ConstMethod *)poolTable[poolIndex].value;
    }
    throw "index for const method is invalid";
}

const ConstMethod &ClassLoader::getConstMethod(const ConstPool &constPool) const {
    return getConstMethod((uint16_t)(&constPool - poolTable) + 1);
}

const ConstInterfaceMethod &ClassLoader::getConstInterfaceMethod(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_INTERFACE_METHOD) {
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_INTERFACE_METHOD;
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(ConstInterfaceMethod));
            new ((ConstInterfaceMethod *)poolTable[poolIndex].value)ConstInterfaceMethod(getConstClass(classNameIndex), getConstNameAndType(nameAndTypeIndex));
        }
        return *(ConstInterfaceMethod *)poolTable[poolIndex].value;
    }
    throw "index for const interface method is invalid";
}

const ConstInterfaceMethod &ClassLoader::getConstInterfaceMethod(const ConstPool &constPool) const {
    return getConstInterfaceMethod((uint16_t)(&constPool - poolTable) + 1);
}

ClassAccessFlag ClassLoader::getAccessFlag(void) const {
    return (ClassAccessFlag)accessFlags;
}

const ConstUtf8 &ClassLoader::getThisClass(void) const {
    return getConstClass(thisClass);
}

const ConstUtf8 &ClassLoader::getSuperClass(void) const {
    if(superClass == 0)
        return *(ConstUtf8 *)0;
    return getConstClass(superClass);
}

uint16_t ClassLoader::getInterfacesCount(void) const {
    return interfacesCount;
}

const ConstUtf8 &ClassLoader::getInterface(uint8_t interfaceIndex) const {
    if(interfaceIndex < interfacesCount)
        return getConstClass(interfaces[interfaceIndex]);
    throw "index for const interface is invalid";
}

uint16_t ClassLoader::getFieldsCount(void) const {
    return fieldsCount;
}

const FieldInfo &ClassLoader::getFieldInfo(uint8_t fieldIndex) const {
    if(fieldIndex < fieldsCount)
        return fields[fieldIndex];
    throw "index for field info is invalid";
}

const FieldInfo &ClassLoader::getFieldInfo(const ConstNameAndType &fieldName) const {
    for(uint16_t i = 0; i < fieldsCount; i++) {
        if(&fieldName.name == &fields[i].name && &fieldName.descriptor == &fields[i].descriptor)
            return fields[i];
        else if(fieldName.name == fields[i].name && fieldName.descriptor == fields[i].descriptor)
            return fields[i];
    }
    return *(const FieldInfo *)0;
}

uint16_t ClassLoader::getMethodsCount(void) const {
    return methodsCount;
}

const MethodInfo &ClassLoader::getMethodInfo(uint8_t methodIndex) const {
    if(methodIndex < methodsCount)
        return methods[methodIndex];
    throw "index for method info is invalid";
}

const MethodInfo &ClassLoader::getMethodInfo(const ConstNameAndType &methodName) const {
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(&methodName.name == &methods[i].name && &methodName.descriptor == &methods[i].descriptor)
            return methods[i];
        else if(methodName.name == methods[i].name && methodName.descriptor == methods[i].descriptor)
            return methods[i];
    }
    return *(const MethodInfo *)0;
}

const MethodInfo &ClassLoader::getMainMethodInfo(void) const {
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(methods[i].name.length == (sizeof("main") - 1) && methods[i].descriptor.length >= sizeof("([Ljava/lang/String;)")) {
            if(
                strncmp(methods[i].name.text, "main", sizeof("main") - 1) == 0 &&
                strncmp(methods[i].descriptor.text, "([Ljava/lang/String;)V", sizeof("([Ljava/lang/String;)") - 1) == 0
            ) {
                if((methods[i].accessFlag & (METHOD_PUBLIC | METHOD_STATIC)) == (METHOD_PUBLIC | METHOD_STATIC))
                    return methods[i];
                throw "main must be static public method";
            }
        }
    }
    return *(const MethodInfo *)0;
}

const MethodInfo &ClassLoader::getStaticConstructor(void) const {
    for(int32_t i = methodsCount - 1; i >= 0; i--) {
        if(
            methods[i].accessFlag == METHOD_STATIC &&
            methods[i].name.length == (sizeof("<clinit>") - 1) &&
            methods[i].descriptor.length == (sizeof("()V") - 1) &&
            strncmp(methods[i].name.text, "<clinit>", sizeof("<clinit>") - 1) == 0 &&
            strncmp(methods[i].descriptor.text, "()V", sizeof("()V") - 1) == 0
        ) {
            return methods[i];
        }
    }
    return *(MethodInfo *)0;
}

ClassLoader::~ClassLoader(void) {
    if(poolCount) {
        for(uint32_t i = 0; i < poolCount; i++) {
            switch (poolTable[i].tag) {
                case CONST_UTF8:
                case CONST_FIELD:
                case CONST_METHOD:
                case CONST_INTERFACE_METHOD:
                case CONST_NAME_AND_TYPE:
                case CONST_INVOKE_DYNAMIC:
                    Mjvm::free((void *)poolTable[i].value);
                    break;
                case CONST_LONG:
                case CONST_DOUBLE:
                    i++;
                    break;
                case CONST_METHOD_HANDLE:
                    Mjvm::free((void *)poolTable[i].value);
                    break;
                default:
                    break;
            }
        }
        Mjvm::free(poolTable);
    }
    if(interfacesCount)
        Mjvm::free(interfaces);
    if(fieldsCount) {
        for(uint32_t i = 0; i < fieldsCount; i++)
            fields[i].~FieldInfo();
        Mjvm::free(fields);
    }
    if(methodsCount) {
        for(uint32_t i = 0; i < methodsCount; i++)
            methods[i].~MethodInfo();
        Mjvm::free(methods);
    }
    for(AttributeInfo *node = attributes; node != 0;) {
        AttributeInfo *next = node->next;
        node->~AttributeInfo();
        Mjvm::free(node);
        node = next;
    }
    Mjvm::free(attributes);
}
