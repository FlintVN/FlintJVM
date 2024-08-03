
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_opcodes.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"

#if __has_include("flint_conf.h")
#include "flint_conf.h"
#endif
#include "flint_default_conf.h"

typedef struct {
    FlintConstUtf8 *constUtf8Class;
    FlintClass *constClass;
} ConstClassValue;

static void *ClassLoader_Open(const char *fileName) {
    char buff[FILE_NAME_BUFF_SIZE];
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

    void *file = FlintAPI::File::open(buff, FLINT_FILE_READ);
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

    void *file = FlintAPI::File::open(buff, FLINT_FILE_READ);
    if(file == 0)
        throw "can not open file";
    return file;
}

static void ClassLoader_Read(void *file, void *buff, uint32_t size) {
    uint32_t temp;
    FlintFileResult ret = FlintAPI::File::read(file, buff, size, &temp);
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
    return Flint_Swap16(temp);
}

static uint32_t ClassLoader_ReadUInt32(void *file) {
    uint32_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return Flint_Swap32(temp);
}

static uint64_t ClassLoader_ReadUInt64(void *file) {
    uint64_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return Flint_Swap64(temp);
}

static void ClassLoader_Seek(void *file, int32_t offset) {
    if(FlintAPI::File::seek(file, FlintAPI::File::tell(file) + offset) != FILE_RESULT_OK)
        throw "read file error";
}

FlintClassLoader::FlintClassLoader(const char *fileName) {
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
        FlintAPI::File::close(file);
        throw excp;
    }

    FlintAPI::File::close(file);
}

FlintClassLoader::FlintClassLoader(const char *fileName, uint16_t length) {
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
        FlintAPI::File::close(file);
        throw excp;
    }

    FlintAPI::File::close(file);
}

FlintClassLoader::FlintClassLoader(const FlintConstUtf8 &fileName) : FlintClassLoader(fileName.text, fileName.length) {

}

void FlintClassLoader::readFile(void *file) {
    magic = ClassLoader_ReadUInt32(file);
    minorVersion = ClassLoader_ReadUInt16(file);
    majorVersion = ClassLoader_ReadUInt16(file);
    poolCount = ClassLoader_ReadUInt16(file) - 1;
    poolTable = (FlintConstPool *)Flint::malloc(poolCount * sizeof(FlintConstPool));
    for(uint32_t i = 0; i < poolCount; i++) {
        *(FlintConstPoolTag *)&poolTable[i].tag = (FlintConstPoolTag)ClassLoader_ReadUInt8(file);
        switch(poolTable[i].tag) {
            case CONST_UTF8: {
                uint16_t length = ClassLoader_ReadUInt16(file);
                *(uint32_t *)&poolTable[i].value = (uint32_t)Flint::malloc(sizeof(FlintConstUtf8) + length + 1);
                *(uint16_t *)&((FlintConstUtf8 *)poolTable[i].value)->length = length;
                char *textBuff = (char *)((FlintConstUtf8 *)poolTable[i].value)->text;
                ClassLoader_Read(file, textBuff, length);
                *(uint16_t *)&((FlintConstUtf8 *)poolTable[i].value)->crc = Flint_CalcCrc((uint8_t *)textBuff, length);
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
                *(FlintConstPoolTag *)&poolTable[i + 1].tag = CONST_UNKOWN;
                i++;
                break;
            }
            case CONST_CLASS:
            case CONST_STRING:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
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
        interfaces = (uint16_t *)Flint::malloc(interfacesCount * sizeof(uint16_t));
        ClassLoader_Read(file, interfaces, interfacesCount * sizeof(uint16_t));
    }
    fieldsCount = ClassLoader_ReadUInt16(file);
    if(fieldsCount) {
        uint32_t loadedCount = 0;
        fields = (FlintFieldInfo *)Flint::malloc(fieldsCount * sizeof(FlintFieldInfo));
        for(uint16_t i = 0; i < fieldsCount; i++) {
            FlintFieldAccessFlag flag = (FlintFieldAccessFlag)ClassLoader_ReadUInt16(file);
            uint16_t fieldsNameIndex = ClassLoader_ReadUInt16(file);
            uint16_t fieldsDescriptorIndex = ClassLoader_ReadUInt16(file);
            uint16_t fieldsAttributesCount = ClassLoader_ReadUInt16(file);
            while(fieldsAttributesCount--) {
                uint16_t nameIndex = ClassLoader_ReadUInt16(file);
                uint32_t length = ClassLoader_ReadUInt32(file);
                FlintAttributeType type = FlintAttribute::parseAttributeType(getConstUtf8(nameIndex));
                if(
                    type == ATTRIBUTE_CONSTANT_VALUE &&
                    (FlintFieldAccessFlag)(flag & (FIELD_STATIC | FIELD_FINAL)) == (FIELD_STATIC | FIELD_FINAL)
                ) {
                    flag = (FlintFieldAccessFlag)(flag | FIELD_UNLOAD);
                }
                ClassLoader_Seek(file, length);
            }
            if(!(flag & FIELD_UNLOAD)) {
                new (&fields[loadedCount])FlintFieldInfo(*this, flag, getConstUtf8(fieldsNameIndex), getConstUtf8(fieldsDescriptorIndex));
                loadedCount++;
            }
        }
        if(loadedCount == 0) {
            Flint::free(fields);
            fields = 0;
            fieldsCount = 0;
        }
        else if(loadedCount != fieldsCount) {
            fields = (FlintFieldInfo *)Flint::realloc(fields, loadedCount * sizeof(FlintFieldInfo));
            fieldsCount = loadedCount;
        }
    }
    methodsCount = ClassLoader_ReadUInt16(file);
    if(methodsCount) {
        uint32_t loadedCount = 0;
        methods = (FlintMethodInfo *)Flint::malloc(methodsCount * sizeof(FlintMethodInfo));
        for(uint16_t i = 0; i < methodsCount; i++) {
            FlintMethodAccessFlag flag = (FlintMethodAccessFlag)ClassLoader_ReadUInt16(file);
            uint16_t methodNameIndex = ClassLoader_ReadUInt16(file);
            uint16_t methodDescriptorIndex = ClassLoader_ReadUInt16(file);
            uint16_t methodAttributesCount = ClassLoader_ReadUInt16(file);
            if((flag & METHOD_BRIDGE) == METHOD_BRIDGE) {
                while(methodAttributesCount--)
                    readAttribute(file, true);
            }
            else {
                new (&methods[loadedCount])FlintMethodInfo(*this, flag, getConstUtf8(methodNameIndex), getConstUtf8(methodDescriptorIndex));
                if((flag & METHOD_NATIVE) == METHOD_NATIVE) {
                    FlintNativeAttribute *attrNative = (FlintNativeAttribute *)Flint::malloc(sizeof(FlintNativeAttribute));
                    new (attrNative)FlintNativeAttribute(0);
                    methods[loadedCount].addAttribute(attrNative);
                }
                while(methodAttributesCount--) {
                    FlintAttribute *attr = readAttribute(file);
                    if(attr != 0)
                        methods[loadedCount].addAttribute(attr);
                }
                loadedCount++;
            }
        }
        if(loadedCount == 0) {
            Flint::free(methods);
            methods = 0;
            methodsCount = 0;
        }
        else if(loadedCount != methodsCount) {
            methods = (FlintMethodInfo *)Flint::realloc(methods, loadedCount * sizeof(FlintMethodInfo));
            methodsCount = loadedCount;
        }
    }
    attributesCount = ClassLoader_ReadUInt16(file);
    while(attributesCount--) {
        FlintAttribute *attr = readAttribute(file);
        if(attr)
            addAttribute(attr);
    }
}

void FlintClassLoader::addAttribute(FlintAttribute *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

FlintAttribute *FlintClassLoader::readAttribute(void *file, bool isDummy) {
    uint16_t nameIndex = ClassLoader_ReadUInt16(file);
    uint32_t length = ClassLoader_ReadUInt32(file);
    if(isDummy) {
        ClassLoader_Seek(file, length);
        return 0;
    }
    FlintAttributeType type = FlintAttribute::parseAttributeType(getConstUtf8(nameIndex));
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

FlintAttribute *FlintClassLoader::readAttributeCode(void *file) {
    FlintCodeAttribute *attribute = (FlintCodeAttribute *)Flint::malloc(sizeof(FlintCodeAttribute));
    uint16_t maxStack = ClassLoader_ReadUInt16(file);
    uint16_t maxLocals = ClassLoader_ReadUInt16(file);
    uint32_t codeLength = ClassLoader_ReadUInt32(file);
    uint8_t *code = (uint8_t *)Flint::malloc(codeLength + 1);
    new (attribute)FlintCodeAttribute(maxStack, maxLocals);
    ClassLoader_Read(file, code, codeLength);
    code[codeLength] = OP_EXIT;
    attribute->setCode(code, codeLength);
    uint16_t exceptionTableLength = ClassLoader_ReadUInt16(file);
    if(exceptionTableLength) {
        FlintExceptionTable *exceptionTable = (FlintExceptionTable *)Flint::malloc(exceptionTableLength * sizeof(FlintExceptionTable));
        attribute->setExceptionTable(exceptionTable, exceptionTableLength);
        for(uint16_t i = 0; i < exceptionTableLength; i++) {
            uint16_t startPc = ClassLoader_ReadUInt16(file);
            uint16_t endPc = ClassLoader_ReadUInt16(file);
            uint16_t handlerPc = ClassLoader_ReadUInt16(file);
            uint16_t catchType = ClassLoader_ReadUInt16(file);
            new (&exceptionTable[i])FlintExceptionTable(startPc, endPc, handlerPc, catchType);
        }
    }
    uint16_t attrbutesCount = ClassLoader_ReadUInt16(file);
    while(attrbutesCount--) {
        FlintAttribute *attr = readAttribute(file);
        if(attr)
            attribute->addAttribute(attr);
    }
    return attribute;
}

FlintAttribute *FlintClassLoader::readAttributeBootstrapMethods(void *file) {
    uint16_t numBootstrapMethods = ClassLoader_ReadUInt16(file);
    AttributeBootstrapMethods *attribute = (AttributeBootstrapMethods *)Flint::malloc(sizeof(AttributeBootstrapMethods));
    new (attribute)AttributeBootstrapMethods(numBootstrapMethods);
    for(uint16_t i = 0; i < numBootstrapMethods; i++) {
        uint16_t bootstrapMethodRef = ClassLoader_ReadUInt16(file);
        uint16_t numBootstrapArguments = ClassLoader_ReadUInt16(file);
        FlintBootstrapMethod *bootstrapMethod = (FlintBootstrapMethod *)Flint::malloc(sizeof(FlintBootstrapMethod) + numBootstrapArguments * sizeof(uint16_t));
        new (bootstrapMethod)FlintBootstrapMethod(bootstrapMethodRef, numBootstrapArguments);
        uint16_t *bootstrapArguments = (uint16_t *)(((uint8_t *)bootstrapMethod) + sizeof(FlintBootstrapMethod));
        ClassLoader_Read(file, bootstrapArguments, numBootstrapArguments * sizeof(uint16_t));
        attribute->setBootstrapMethod(i, *bootstrapMethod);
    }
    return attribute;
}

uint32_t FlintClassLoader::getMagic(void) const {
    return magic;
}

uint16_t FlintClassLoader::getMinorVersion(void) const {
    return minorVersion;
}

uint16_t FlintClassLoader::getMajorversion(void) const {
    return majorVersion;
}

FlintConstPool &FlintClassLoader::getConstPool(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount)
        return poolTable[poolIndex];
    throw "index for const pool is invalid";
}

int32_t FlintClassLoader::getConstInteger(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_INTEGER)
        return (int32_t)poolTable[poolIndex].value;
    throw "index for const integer is invalid";
}

int32_t FlintClassLoader::getConstInteger(FlintConstPool &constPool) const {
    if(constPool.tag == CONST_INTEGER)
        return (int32_t)constPool.value;
    throw "const pool tag is not integer tag";
}

float FlintClassLoader::getConstFloat(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_FLOAT)
        return *(float *)&poolTable[poolIndex].value;
    throw "index for const float is invalid";
}

float FlintClassLoader::getConstFloat(FlintConstPool &constPool) const {
    if(constPool.tag == CONST_FLOAT)
        return *(float *)&constPool.value;
    throw "const pool tag is not float tag";
}

int64_t FlintClassLoader::getConstLong(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_LONG)
        return ((uint64_t)poolTable[poolIndex + 1].value << 32) | poolTable[poolIndex].value;
    throw "index for const long is invalid";
}

int64_t FlintClassLoader::getConstLong(FlintConstPool &constPool) const {
    return getConstLong((uint16_t)(&constPool - poolTable) + 1);
}

double FlintClassLoader::getConstDouble(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_DOUBLE) {
        uint64_t ret = ((uint64_t)poolTable[poolIndex + 1].value << 32) | poolTable[poolIndex].value;
        return *(double *)&ret;
    }
    throw "index for const double is invalid";
}

double FlintClassLoader::getConstDouble(FlintConstPool &constPool) const {
    return getConstDouble((uint16_t)(&constPool - poolTable) + 1);
}

FlintConstUtf8 &FlintClassLoader::getConstUtf8(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_UTF8)
        return *(FlintConstUtf8 *)poolTable[poolIndex].value;
    throw "index for const utf8 is invalid";
}

FlintConstUtf8 &FlintClassLoader::getConstUtf8(FlintConstPool &constPool) const {
    if(constPool.tag == CONST_UTF8)
        return *(FlintConstUtf8 *)constPool.value;
    throw "const pool tag is not utf8 tag";
}

FlintConstUtf8 &FlintClassLoader::getConstUtf8Class(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_CLASS) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t index = poolTable[poolIndex].value;
                Flint::unlock();
                return getConstUtf8(index);
            }
            Flint::unlock();
        }
        return *((ConstClassValue *)poolTable[poolIndex].value)->constUtf8Class;
    }
    throw "index for const class is invalid";
}

FlintConstUtf8 &FlintClassLoader::getConstUtf8Class(FlintConstPool &constPool) const {
    if((constPool.tag & 0x7F) == CONST_CLASS) {
        if(constPool.tag & 0x80) {
            Flint::lock();
            if(constPool.tag & 0x80) {
                uint16_t index = constPool.value;
                Flint::unlock();
                return getConstUtf8(index);
            }
            Flint::unlock();
        }
        return *((ConstClassValue *)constPool.value)->constUtf8Class;
    }
    throw "const pool tag is not class tag";
}

FlintClass &FlintClassLoader::getConstClass(Flint &flint, uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_CLASS) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                FlintConstUtf8 &constUtf8Class = getConstUtf8(poolTable[poolIndex].value);
                ConstClassValue *constClassValue = (ConstClassValue *)Flint::malloc(sizeof(ConstClassValue));
                constClassValue->constUtf8Class = &constUtf8Class;
                constClassValue->constClass = flint.getConstClass(constUtf8Class.text, constUtf8Class.length);
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)constClassValue;
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_CLASS;
            }
            Flint::unlock();
        }
        return *((ConstClassValue *)poolTable[poolIndex].value)->constClass;
    }
    throw "index for const class is invalid";
}

FlintClass &FlintClassLoader::getConstClass(Flint &flint, FlintConstPool &constPool) {
    if((constPool.tag & 0x7F) == CONST_CLASS) {
        if(constPool.tag & 0x80) {
            Flint::lock();
            if(constPool.tag & 0x80) {
                FlintConstUtf8 &constUtf8Class = getConstUtf8(constPool.value);
                ConstClassValue *constClassValue = (ConstClassValue *)Flint::malloc(sizeof(ConstClassValue));
                constClassValue->constUtf8Class = &constUtf8Class;
                constClassValue->constClass = flint.getConstClass(constUtf8Class.text, constUtf8Class.length);
                *(uint32_t *)&constPool.value = (uint32_t)constClassValue;
                *(FlintConstPoolTag *)&constPool.tag = CONST_CLASS;
            }
            Flint::unlock();
        }
        return *((ConstClassValue *)constPool.value)->constClass;
    }
    throw "const pool tag is not class tag";
}

FlintString &FlintClassLoader::getConstString(Flint &flint, uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_STRING) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                FlintConstUtf8 &utf8Str = getConstUtf8(poolTable[poolIndex].value);
                FlintString *strObj = flint.getConstString(utf8Str);
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)strObj;
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_STRING;
            }
            Flint::unlock();
        }
        return *(FlintString *)poolTable[poolIndex].value;
    }
    throw "index for const string is invalid";
}

FlintString &FlintClassLoader::getConstString(Flint &flint, FlintConstPool &constPool) {
    if((constPool.tag & 0x7F) == CONST_STRING) {
        if(constPool.tag & 0x80) {
            Flint::lock();
            if(constPool.tag & 0x80) {
                FlintConstUtf8 &utf8Str = getConstUtf8(constPool.value);
                FlintString *strObj = flint.getConstString(utf8Str);
                *(uint32_t *)&constPool.value = (uint32_t)strObj;
                *(FlintConstPoolTag *)&constPool.tag = CONST_STRING;
            }
            Flint::unlock();
        }
        return *(FlintString *)constPool.value;
    }
    throw "const pool tag is not string tag";
}

FlintConstUtf8 &FlintClassLoader::getConstMethodType(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_METHOD)
        return getConstUtf8(poolTable[poolIndex].value);
    throw "index for const method type is invalid";
}

FlintConstUtf8 &FlintClassLoader::getConstMethodType(FlintConstPool &constPool) const {
    if(constPool.tag == CONST_METHOD)
        return getConstUtf8(constPool.value);
    throw "const pool tag is not method type tag";
}

FlintConstNameAndType &FlintClassLoader::getConstNameAndType(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_NAME_AND_TYPE) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t nameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t descriptorIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_NAME_AND_TYPE;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstNameAndType));
                new ((FlintConstNameAndType *)poolTable[poolIndex].value)FlintConstNameAndType(getConstUtf8(nameIndex), getConstUtf8(descriptorIndex));
            }
            Flint::unlock();
        }
        return *(FlintConstNameAndType *)poolTable[poolIndex].value;
    }
    throw "index for const name and type is invalid";
}

FlintConstNameAndType &FlintClassLoader::getConstNameAndType(FlintConstPool &constPool) {
    return getConstNameAndType((uint16_t)(&constPool - poolTable) + 1);
}

FlintConstField &FlintClassLoader::getConstField(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_FIELD) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_FIELD;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstField));
                new ((FlintConstField *)poolTable[poolIndex].value)FlintConstField(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
            }
            Flint::unlock();
        }
        return *(FlintConstField *)poolTable[poolIndex].value;
    }
    throw "index for const field is invalid";
}

FlintConstField &FlintClassLoader::getConstField(FlintConstPool &constPool) {
    return getConstField((uint16_t)(&constPool - poolTable) + 1);
}

FlintConstMethod &FlintClassLoader::getConstMethod(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_METHOD) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_METHOD;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstMethod));
                new ((FlintConstMethod *)poolTable[poolIndex].value)FlintConstMethod(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
            }
            Flint::unlock();
        }
        return *(FlintConstMethod *)poolTable[poolIndex].value;
    }
    throw "index for const method is invalid";
}

FlintConstMethod &FlintClassLoader::getConstMethod(FlintConstPool &constPool) {
    return getConstMethod((uint16_t)(&constPool - poolTable) + 1);
}

FlintConstInterfaceMethod &FlintClassLoader::getConstInterfaceMethod(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_INTERFACE_METHOD) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_INTERFACE_METHOD;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstInterfaceMethod));
                new ((FlintConstInterfaceMethod *)poolTable[poolIndex].value)FlintConstInterfaceMethod(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
            }
            Flint::unlock();
        }
        return *(FlintConstInterfaceMethod *)poolTable[poolIndex].value;
    }
    throw "index for const interface method is invalid";
}

FlintConstInterfaceMethod &FlintClassLoader::getConstInterfaceMethod(FlintConstPool &constPool) {
    return getConstInterfaceMethod((uint16_t)(&constPool - poolTable) + 1);
}

FlintClassAccessFlag FlintClassLoader::getAccessFlag(void) const {
    return (FlintClassAccessFlag)accessFlags;
}

FlintConstUtf8 &FlintClassLoader::getThisClass(void) const {
    return getConstUtf8Class(thisClass);
}

FlintConstUtf8 &FlintClassLoader::getSuperClass(void) const {
    if(superClass == 0)
        return *(FlintConstUtf8 *)0;
    return getConstUtf8Class(superClass);
}

uint16_t FlintClassLoader::getInterfacesCount(void) const {
    return interfacesCount;
}

FlintConstUtf8 &FlintClassLoader::getInterface(uint8_t interfaceIndex) const {
    if(interfaceIndex < interfacesCount)
        return getConstUtf8Class(interfaces[interfaceIndex]);
    throw "index for const interface is invalid";
}

uint16_t FlintClassLoader::getFieldsCount(void) const {
    return fieldsCount;
}

FlintFieldInfo &FlintClassLoader::getFieldInfo(uint8_t fieldIndex) const {
    if(fieldIndex < fieldsCount)
        return fields[fieldIndex];
    throw "index for field info is invalid";
}

FlintFieldInfo &FlintClassLoader::getFieldInfo(FlintConstUtf8 &name, FlintConstUtf8 &descriptor) const {
    uint32_t nameHash = CONST_UTF8_HASH(name);
    uint32_t descriptorHash = CONST_UTF8_HASH(descriptor);
    for(uint16_t i = 0; i < fieldsCount; i++) {
        if(nameHash == CONST_UTF8_HASH(fields[i].name) && descriptorHash == CONST_UTF8_HASH(fields[i].descriptor)) {
            if(&name == &fields[i].name && &descriptor == &fields[i].descriptor)
                return fields[i];
            else if(
                strncmp(name.text, fields[i].name.text, name.length) == 0 &&
                strncmp(descriptor.text, fields[i].descriptor.text, descriptor.length) == 0
            ) {
                return fields[i];
            }
        }
    }
    return *(FlintFieldInfo *)0;
}

FlintFieldInfo &FlintClassLoader::getFieldInfo(FlintConstNameAndType &nameAndType) const {
    return getFieldInfo(nameAndType.name, nameAndType.descriptor);
}

uint16_t FlintClassLoader::getMethodsCount(void) const {
    return methodsCount;
}

FlintMethodInfo &FlintClassLoader::getMethodInfo(uint8_t methodIndex) const {
    if(methodIndex < methodsCount)
        return methods[methodIndex];
    throw "index for method info is invalid";
}

FlintMethodInfo &FlintClassLoader::getMethodInfo(FlintConstUtf8 &name, FlintConstUtf8 &descriptor) const {
    uint32_t nameHash = CONST_UTF8_HASH(name);
    uint32_t descriptorHash = CONST_UTF8_HASH(descriptor);
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(nameHash == CONST_UTF8_HASH(methods[i].name) && descriptorHash == CONST_UTF8_HASH(methods[i].descriptor)) {
            if(&name == &methods[i].name && &descriptor == &methods[i].descriptor)
                return methods[i];
            else if(
                strncmp(name.text, methods[i].name.text, name.length) == 0 &&
                strncmp(descriptor.text, methods[i].descriptor.text, descriptor.length) == 0
            ) {
                return methods[i];
            }
        }
    }
    return *(FlintMethodInfo *)0;
}

FlintMethodInfo &FlintClassLoader::getMethodInfo(FlintConstNameAndType &nameAndType) const {
    return getMethodInfo(nameAndType.name, nameAndType.descriptor);
}

FlintMethodInfo &FlintClassLoader::getMainMethodInfo(void) const {
    static const uint32_t nameAndType[] = {
        (uint32_t)"\x04\x00\xA5\x01""main",                     /* method name */
        (uint32_t)"\x16\x00\xA2\x07""([Ljava/lang/String;)V",   /* method type */
    };
    return getMethodInfo(*(FlintConstNameAndType *)nameAndType);
}

FlintMethodInfo &FlintClassLoader::getStaticConstructor(void) const {
    static const uint32_t nameAndType[] = {
        (uint32_t)"\x08\x00\xFD\x02""<clinit>",                 /* method name */
        (uint32_t)"\x03\x00\xA7\x00""()V",                      /* method type */
    };
    return getMethodInfo(*(FlintConstNameAndType *)nameAndType);
}

FlintClassLoader::~FlintClassLoader(void) {
    if(poolCount) {
        for(uint32_t i = 0; i < poolCount; i++) {
            switch (poolTable[i].tag) {
                case CONST_UTF8:
                case CONST_FIELD:
                case CONST_METHOD:
                case CONST_INTERFACE_METHOD:
                case CONST_NAME_AND_TYPE:
                case CONST_INVOKE_DYNAMIC:
                    Flint::free((void *)poolTable[i].value);
                    break;
                case CONST_LONG:
                case CONST_DOUBLE:
                    i++;
                    break;
                case CONST_CLASS:
                case CONST_METHOD_HANDLE:
                    Flint::free((void *)poolTable[i].value);
                    break;
                default:
                    break;
            }
        }
        Flint::free(poolTable);
    }
    if(interfacesCount)
        Flint::free(interfaces);
    if(fieldsCount) {
        for(uint32_t i = 0; i < fieldsCount; i++)
            fields[i].~FlintFieldInfo();
        Flint::free(fields);
    }
    if(methodsCount) {
        for(uint32_t i = 0; i < methodsCount; i++)
            methods[i].~FlintMethodInfo();
        Flint::free(methods);
    }
    for(FlintAttribute *node = attributes; node != 0;) {
        FlintAttribute *next = node->next;
        node->~FlintAttribute();
        Flint::free(node);
        node = next;
    }
    Flint::free(attributes);
}
