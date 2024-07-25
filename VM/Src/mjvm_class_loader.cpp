
#include <iostream>
#include <string.h>
#include "mjvm.h"
#include "mjvm_opcodes.h"
#include "mjvm_system_api.h"
#include "mjvm_class_loader.h"

#if __has_include("mjvm_conf.h")
#include "mjvm_conf.h"
#endif
#include "mjvm_default_conf.h"

typedef struct {
    MjvmConstUtf8 *constUtf8Class;
    MjvmClass *constClass;
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
    return Mjvm_Swap16(temp);
}

static uint32_t ClassLoader_ReadUInt32(void *file) {
    uint32_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return Mjvm_Swap32(temp);
}

static uint64_t ClassLoader_ReadUInt64(void *file) {
    uint64_t temp;
    ClassLoader_Read(file, &temp, sizeof(temp));
    return Mjvm_Swap64(temp);
}

static void ClassLoader_Seek(void *file, int32_t offset) {
    if(MjvmSystem_FileSeek(file, MjvmSystem_FileTell(file) + offset) != FILE_RESULT_OK)
        throw "read file error";
}

MjvmClassLoader::MjvmClassLoader(const char *fileName) {
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

MjvmClassLoader::MjvmClassLoader(const char *fileName, uint16_t length) {
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

MjvmClassLoader::MjvmClassLoader(const MjvmConstUtf8 &fileName) : MjvmClassLoader(fileName.text, fileName.length) {

}

void MjvmClassLoader::readFile(void *file) {
    magic = ClassLoader_ReadUInt32(file);
    minorVersion = ClassLoader_ReadUInt16(file);
    majorVersion = ClassLoader_ReadUInt16(file);
    poolCount = ClassLoader_ReadUInt16(file) - 1;
    poolTable = (MjvmConstPool *)Mjvm::malloc(poolCount * sizeof(MjvmConstPool));
    for(uint32_t i = 0; i < poolCount; i++) {
        *(MjvmConstPoolTag *)&poolTable[i].tag = (MjvmConstPoolTag)ClassLoader_ReadUInt8(file);
        switch(poolTable[i].tag) {
            case CONST_UTF8: {
                uint16_t length = ClassLoader_ReadUInt16(file);
                *(uint32_t *)&poolTable[i].value = (uint32_t)Mjvm::malloc(sizeof(MjvmConstUtf8) + length + 1);
                *(uint16_t *)&((MjvmConstUtf8 *)poolTable[i].value)->length = length;
                char *textBuff = (char *)((MjvmConstUtf8 *)poolTable[i].value)->text;
                ClassLoader_Read(file, textBuff, length);
                *(uint16_t *)&((MjvmConstUtf8 *)poolTable[i].value)->crc = Mjvm_CalcCrc((uint8_t *)textBuff, length);
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
                *(MjvmConstPoolTag *)&poolTable[i + 1].tag = CONST_UNKOWN;
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
        interfaces = (uint16_t *)Mjvm::malloc(interfacesCount * sizeof(uint16_t));
        ClassLoader_Read(file, interfaces, interfacesCount * sizeof(uint16_t));
    }
    fieldsCount = ClassLoader_ReadUInt16(file);
    if(fieldsCount) {
        uint32_t loadedCount = 0;
        fields = (MjvmFieldInfo *)Mjvm::malloc(fieldsCount * sizeof(MjvmFieldInfo));
        for(uint16_t i = 0; i < fieldsCount; i++) {
            MjvmFieldAccessFlag flag = (MjvmFieldAccessFlag)ClassLoader_ReadUInt16(file);
            uint16_t fieldsNameIndex = ClassLoader_ReadUInt16(file);
            uint16_t fieldsDescriptorIndex = ClassLoader_ReadUInt16(file);
            uint16_t fieldsAttributesCount = ClassLoader_ReadUInt16(file);
            while(fieldsAttributesCount--) {
                uint16_t nameIndex = ClassLoader_ReadUInt16(file);
                uint32_t length = ClassLoader_ReadUInt32(file);
                MjvmAttributeType type = MjvmAttribute::parseAttributeType(getConstUtf8(nameIndex));
                if(type == ATTRIBUTE_CONSTANT_VALUE)
                    flag = (MjvmFieldAccessFlag)(flag | FIELD_UNLOAD);
                ClassLoader_Seek(file, length);
            }
            if(!(flag & FIELD_UNLOAD)) {
                new (&fields[loadedCount])MjvmFieldInfo(*this, flag, getConstUtf8(fieldsNameIndex), getConstUtf8(fieldsDescriptorIndex));
                loadedCount++;
            }
        }
        if(loadedCount == 0) {
            Mjvm::free(fields);
            fields = 0;
            fieldsCount = 0;
        }
        else if(loadedCount != fieldsCount) {
            fields = (MjvmFieldInfo *)Mjvm::realloc(fields, loadedCount * sizeof(MjvmFieldInfo));
            fieldsCount = loadedCount;
        }
    }
    methodsCount = ClassLoader_ReadUInt16(file);
    if(methodsCount) {
        uint32_t loadedCount = 0;
        methods = (MjvmMethodInfo *)Mjvm::malloc(methodsCount * sizeof(MjvmMethodInfo));
        for(uint16_t i = 0; i < methodsCount; i++) {
            MjvmMethodAccessFlag flag = (MjvmMethodAccessFlag)ClassLoader_ReadUInt16(file);
            uint16_t methodNameIndex = ClassLoader_ReadUInt16(file);
            uint16_t methodDescriptorIndex = ClassLoader_ReadUInt16(file);
            uint16_t methodAttributesCount = ClassLoader_ReadUInt16(file);
            if((flag & METHOD_BRIDGE) == METHOD_BRIDGE) {
                while(methodAttributesCount--)
                    readAttribute(file, true);
            }
            else {
                new (&methods[loadedCount])MjvmMethodInfo(*this, flag, getConstUtf8(methodNameIndex), getConstUtf8(methodDescriptorIndex));
                if((flag & METHOD_NATIVE) == METHOD_NATIVE) {
                    MjvmNativeAttribute *attrNative = (MjvmNativeAttribute *)Mjvm::malloc(sizeof(MjvmNativeAttribute));
                    new (attrNative)MjvmNativeAttribute(0);
                    methods[loadedCount].addAttribute(attrNative);
                }
                while(methodAttributesCount--) {
                    MjvmAttribute *attr = readAttribute(file);
                    if(attr != 0)
                        methods[loadedCount].addAttribute(attr);
                }
                loadedCount++;
            }
        }
        if(loadedCount == 0) {
            Mjvm::free(methods);
            methods = 0;
            methodsCount = 0;
        }
        else if(loadedCount != methodsCount) {
            methods = (MjvmMethodInfo *)Mjvm::realloc(methods, loadedCount * sizeof(MjvmMethodInfo));
            methodsCount = loadedCount;
        }
    }
    attributesCount = ClassLoader_ReadUInt16(file);
    while(attributesCount--) {
        MjvmAttribute *attr = readAttribute(file);
        if(attr)
            addAttribute(attr);
    }
}

void MjvmClassLoader::addAttribute(MjvmAttribute *attribute) {
    attribute->next = this->attributes;
    this->attributes = attribute;
}

MjvmAttribute *MjvmClassLoader::readAttribute(void *file, bool isDummy) {
    uint16_t nameIndex = ClassLoader_ReadUInt16(file);
    uint32_t length = ClassLoader_ReadUInt32(file);
    if(isDummy) {
        ClassLoader_Seek(file, length);
        return 0;
    }
    MjvmAttributeType type = MjvmAttribute::parseAttributeType(getConstUtf8(nameIndex));
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

MjvmAttribute *MjvmClassLoader::readAttributeCode(void *file) {
    MjvmCodeAttribute *attribute = (MjvmCodeAttribute *)Mjvm::malloc(sizeof(MjvmCodeAttribute));
    uint16_t maxStack = ClassLoader_ReadUInt16(file);
    uint16_t maxLocals = ClassLoader_ReadUInt16(file);
    uint32_t codeLength = ClassLoader_ReadUInt32(file);
    uint8_t *code = (uint8_t *)Mjvm::malloc(codeLength + 1);
    new (attribute)MjvmCodeAttribute(maxStack, maxLocals);
    ClassLoader_Read(file, code, codeLength);
    code[codeLength] = OP_EXIT;
    attribute->setCode(code, codeLength);
    uint16_t exceptionTableLength = ClassLoader_ReadUInt16(file);
    if(exceptionTableLength) {
        MjvmExceptionTable *exceptionTable = (MjvmExceptionTable *)Mjvm::malloc(exceptionTableLength * sizeof(MjvmExceptionTable));
        attribute->setExceptionTable(exceptionTable, exceptionTableLength);
        for(uint16_t i = 0; i < exceptionTableLength; i++) {
            uint16_t startPc = ClassLoader_ReadUInt16(file);
            uint16_t endPc = ClassLoader_ReadUInt16(file);
            uint16_t handlerPc = ClassLoader_ReadUInt16(file);
            uint16_t catchType = ClassLoader_ReadUInt16(file);
            new (&exceptionTable[i])MjvmExceptionTable(startPc, endPc, handlerPc, catchType);
        }
    }
    uint16_t attrbutesCount = ClassLoader_ReadUInt16(file);
    while(attrbutesCount--) {
        MjvmAttribute *attr = readAttribute(file);
        if(attr)
            attribute->addAttribute(attr);
    }
    return attribute;
}

MjvmAttribute *MjvmClassLoader::readAttributeBootstrapMethods(void *file) {
    uint16_t numBootstrapMethods = ClassLoader_ReadUInt16(file);
    AttributeBootstrapMethods *attribute = (AttributeBootstrapMethods *)Mjvm::malloc(sizeof(AttributeBootstrapMethods));
    new (attribute)AttributeBootstrapMethods(numBootstrapMethods);
    for(uint16_t i = 0; i < numBootstrapMethods; i++) {
        uint16_t bootstrapMethodRef = ClassLoader_ReadUInt16(file);
        uint16_t numBootstrapArguments = ClassLoader_ReadUInt16(file);
        MjvmBootstrapMethod *bootstrapMethod = (MjvmBootstrapMethod *)Mjvm::malloc(sizeof(MjvmBootstrapMethod) + numBootstrapArguments * sizeof(uint16_t));
        new (bootstrapMethod)MjvmBootstrapMethod(bootstrapMethodRef, numBootstrapArguments);
        uint16_t *bootstrapArguments = (uint16_t *)(((uint8_t *)bootstrapMethod) + sizeof(MjvmBootstrapMethod));
        ClassLoader_Read(file, bootstrapArguments, numBootstrapArguments * sizeof(uint16_t));
        attribute->setBootstrapMethod(i, *bootstrapMethod);
    }
    return attribute;
}

uint32_t MjvmClassLoader::getMagic(void) const {
    return magic;
}

uint16_t MjvmClassLoader::getMinorVersion(void) const {
    return minorVersion;
}

uint16_t MjvmClassLoader::getMajorversion(void) const {
    return majorVersion;
}

MjvmConstPool &MjvmClassLoader::getConstPool(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount)
        return poolTable[poolIndex];
    throw "index for const pool is invalid";
}

int32_t MjvmClassLoader::getConstInteger(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_INTEGER)
        return (int32_t)poolTable[poolIndex].value;
    throw "index for const integer is invalid";
}

int32_t MjvmClassLoader::getConstInteger(MjvmConstPool &constPool) const {
    if(constPool.tag == CONST_INTEGER)
        return (int32_t)constPool.value;
    throw "const pool tag is not integer tag";
}

float MjvmClassLoader::getConstFloat(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_FLOAT)
        return *(float *)&poolTable[poolIndex].value;
    throw "index for const float is invalid";
}

float MjvmClassLoader::getConstFloat(MjvmConstPool &constPool) const {
    if(constPool.tag == CONST_FLOAT)
        return *(float *)&constPool.value;
    throw "const pool tag is not float tag";
}

int64_t MjvmClassLoader::getConstLong(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_LONG)
        return ((uint64_t)poolTable[poolIndex + 1].value << 32) | poolTable[poolIndex].value;
    throw "index for const long is invalid";
}

int64_t MjvmClassLoader::getConstLong(MjvmConstPool &constPool) const {
    return getConstLong((uint16_t)(&constPool - poolTable) + 1);
}

double MjvmClassLoader::getConstDouble(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_DOUBLE) {
        uint64_t ret = ((uint64_t)poolTable[poolIndex + 1].value << 32) | poolTable[poolIndex].value;
        return *(double *)&ret;
    }
    throw "index for const double is invalid";
}

double MjvmClassLoader::getConstDouble(MjvmConstPool &constPool) const {
    return getConstDouble((uint16_t)(&constPool - poolTable) + 1);
}

MjvmConstUtf8 &MjvmClassLoader::getConstUtf8(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_UTF8)
        return *(MjvmConstUtf8 *)poolTable[poolIndex].value;
    throw "index for const utf8 is invalid";
}

MjvmConstUtf8 &MjvmClassLoader::getConstUtf8(MjvmConstPool &constPool) const {
    if(constPool.tag == CONST_UTF8)
        return *(MjvmConstUtf8 *)constPool.value;
    throw "const pool tag is not utf8 tag";
}

MjvmConstUtf8 &MjvmClassLoader::getConstUtf8Class(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_CLASS) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t index = poolTable[poolIndex].value;
                Mjvm::unlock();
                return getConstUtf8(index);
            }
            Mjvm::unlock();
        }
        return *((ConstClassValue *)poolTable[poolIndex].value)->constUtf8Class;
    }
    throw "index for const class is invalid";
}

MjvmConstUtf8 &MjvmClassLoader::getConstUtf8Class(MjvmConstPool &constPool) const {
    if((constPool.tag & 0x7F) == CONST_CLASS) {
        if(constPool.tag & 0x80) {
            Mjvm::lock();
            if(constPool.tag & 0x80) {
                uint16_t index = constPool.value;
                Mjvm::unlock();
                return getConstUtf8(index);
            }
            Mjvm::unlock();
        }
        return *((ConstClassValue *)constPool.value)->constUtf8Class;
    }
    throw "const pool tag is not class tag";
}

MjvmClass &MjvmClassLoader::getConstClass(Mjvm &mjvm, uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_CLASS) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                MjvmConstUtf8 &constUtf8Class = getConstUtf8(poolTable[poolIndex].value);
                ConstClassValue *constClassValue = (ConstClassValue *)Mjvm::malloc(sizeof(ConstClassValue));
                constClassValue->constUtf8Class = &constUtf8Class;
                constClassValue->constClass = mjvm.getConstClass(constUtf8Class.text, constUtf8Class.length);
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)constClassValue;
                *(MjvmConstPoolTag *)&poolTable[poolIndex].tag = CONST_CLASS;
            }
            Mjvm::unlock();
        }
        return *((ConstClassValue *)poolTable[poolIndex].value)->constClass;
    }
    throw "index for const class is invalid";
}

MjvmClass &MjvmClassLoader::getConstClass(Mjvm &mjvm, MjvmConstPool &constPool) {
    if((constPool.tag & 0x7F) == CONST_CLASS) {
        if(constPool.tag & 0x80) {
            Mjvm::lock();
            if(constPool.tag & 0x80) {
                MjvmConstUtf8 &constUtf8Class = getConstUtf8(constPool.value);
                ConstClassValue *constClassValue = (ConstClassValue *)Mjvm::malloc(sizeof(ConstClassValue));
                constClassValue->constUtf8Class = &constUtf8Class;
                constClassValue->constClass = mjvm.getConstClass(constUtf8Class.text, constUtf8Class.length);
                *(uint32_t *)&constPool.value = (uint32_t)constClassValue;
                *(MjvmConstPoolTag *)&constPool.tag = CONST_CLASS;
            }
            Mjvm::unlock();
        }
        return *((ConstClassValue *)constPool.value)->constClass;
    }
    throw "const pool tag is not class tag";
}

MjvmString &MjvmClassLoader::getConstString(Mjvm &mjvm, uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_STRING) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                MjvmConstUtf8 &utf8Str = getConstUtf8(poolTable[poolIndex].value);
                MjvmString *strObj = mjvm.getConstString(utf8Str);
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)strObj;
                *(MjvmConstPoolTag *)&poolTable[poolIndex].tag = CONST_STRING;
            }
            Mjvm::unlock();
        }
        return *(MjvmString *)poolTable[poolIndex].value;
    }
    throw "index for const string is invalid";
}

MjvmString &MjvmClassLoader::getConstString(Mjvm &mjvm, MjvmConstPool &constPool) {
    if((constPool.tag & 0x7F) == CONST_STRING) {
        if(constPool.tag & 0x80) {
            Mjvm::lock();
            if(constPool.tag & 0x80) {
                MjvmConstUtf8 &utf8Str = getConstUtf8(constPool.value);
                MjvmString *strObj = mjvm.getConstString(utf8Str);
                *(uint32_t *)&constPool.value = (uint32_t)strObj;
                *(MjvmConstPoolTag *)&constPool.tag = CONST_STRING;
            }
            Mjvm::unlock();
        }
        return *(MjvmString *)constPool.value;
    }
    throw "const pool tag is not string tag";
}

MjvmConstUtf8 &MjvmClassLoader::getConstMethodType(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_METHOD)
        return getConstUtf8(poolTable[poolIndex].value);
    throw "index for const method type is invalid";
}

MjvmConstUtf8 &MjvmClassLoader::getConstMethodType(MjvmConstPool &constPool) const {
    if(constPool.tag == CONST_METHOD)
        return getConstUtf8(constPool.value);
    throw "const pool tag is not method type tag";
}

MjvmConstNameAndType &MjvmClassLoader::getConstNameAndType(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_NAME_AND_TYPE) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t nameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t descriptorIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(MjvmConstPoolTag *)&poolTable[poolIndex].tag = CONST_NAME_AND_TYPE;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(MjvmConstNameAndType));
                new ((MjvmConstNameAndType *)poolTable[poolIndex].value)MjvmConstNameAndType(getConstUtf8(nameIndex), getConstUtf8(descriptorIndex));
            }
            Mjvm::unlock();
        }
        return *(MjvmConstNameAndType *)poolTable[poolIndex].value;
    }
    throw "index for const name and type is invalid";
}

MjvmConstNameAndType &MjvmClassLoader::getConstNameAndType(MjvmConstPool &constPool) {
    return getConstNameAndType((uint16_t)(&constPool - poolTable) + 1);
}

MjvmConstField &MjvmClassLoader::getConstField(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_FIELD) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(MjvmConstPoolTag *)&poolTable[poolIndex].tag = CONST_FIELD;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(MjvmConstField));
                new ((MjvmConstField *)poolTable[poolIndex].value)MjvmConstField(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
            }
            Mjvm::unlock();
        }
        return *(MjvmConstField *)poolTable[poolIndex].value;
    }
    throw "index for const field is invalid";
}

MjvmConstField &MjvmClassLoader::getConstField(MjvmConstPool &constPool) {
    return getConstField((uint16_t)(&constPool - poolTable) + 1);
}

MjvmConstMethod &MjvmClassLoader::getConstMethod(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_METHOD) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(MjvmConstPoolTag *)&poolTable[poolIndex].tag = CONST_METHOD;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(MjvmConstMethod));
                new ((MjvmConstMethod *)poolTable[poolIndex].value)MjvmConstMethod(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
            }
            Mjvm::unlock();
        }
        return *(MjvmConstMethod *)poolTable[poolIndex].value;
    }
    throw "index for const method is invalid";
}

MjvmConstMethod &MjvmClassLoader::getConstMethod(MjvmConstPool &constPool) {
    return getConstMethod((uint16_t)(&constPool - poolTable) + 1);
}

MjvmConstInterfaceMethod &MjvmClassLoader::getConstInterfaceMethod(uint16_t poolIndex) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_INTERFACE_METHOD) {
        if(poolTable[poolIndex].tag & 0x80) {
            Mjvm::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                *(MjvmConstPoolTag *)&poolTable[poolIndex].tag = CONST_INTERFACE_METHOD;
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Mjvm::malloc(sizeof(MjvmConstInterfaceMethod));
                new ((MjvmConstInterfaceMethod *)poolTable[poolIndex].value)MjvmConstInterfaceMethod(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
            }
            Mjvm::unlock();
        }
        return *(MjvmConstInterfaceMethod *)poolTable[poolIndex].value;
    }
    throw "index for const interface method is invalid";
}

MjvmConstInterfaceMethod &MjvmClassLoader::getConstInterfaceMethod(MjvmConstPool &constPool) {
    return getConstInterfaceMethod((uint16_t)(&constPool - poolTable) + 1);
}

MjvmClassAccessFlag MjvmClassLoader::getAccessFlag(void) const {
    return (MjvmClassAccessFlag)accessFlags;
}

MjvmConstUtf8 &MjvmClassLoader::getThisClass(void) const {
    return getConstUtf8Class(thisClass);
}

MjvmConstUtf8 &MjvmClassLoader::getSuperClass(void) const {
    if(superClass == 0)
        return *(MjvmConstUtf8 *)0;
    return getConstUtf8Class(superClass);
}

uint16_t MjvmClassLoader::getInterfacesCount(void) const {
    return interfacesCount;
}

MjvmConstUtf8 &MjvmClassLoader::getInterface(uint8_t interfaceIndex) const {
    if(interfaceIndex < interfacesCount)
        return getConstUtf8Class(interfaces[interfaceIndex]);
    throw "index for const interface is invalid";
}

uint16_t MjvmClassLoader::getFieldsCount(void) const {
    return fieldsCount;
}

MjvmFieldInfo &MjvmClassLoader::getFieldInfo(uint8_t fieldIndex) const {
    if(fieldIndex < fieldsCount)
        return fields[fieldIndex];
    throw "index for field info is invalid";
}

MjvmFieldInfo &MjvmClassLoader::getFieldInfo(MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) const {
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
    return *(MjvmFieldInfo *)0;
}

MjvmFieldInfo &MjvmClassLoader::getFieldInfo(MjvmConstNameAndType &nameAndType) const {
    return getFieldInfo(nameAndType.name, nameAndType.descriptor);
}

uint16_t MjvmClassLoader::getMethodsCount(void) const {
    return methodsCount;
}

MjvmMethodInfo &MjvmClassLoader::getMethodInfo(uint8_t methodIndex) const {
    if(methodIndex < methodsCount)
        return methods[methodIndex];
    throw "index for method info is invalid";
}

MjvmMethodInfo &MjvmClassLoader::getMethodInfo(MjvmConstUtf8 &name, MjvmConstUtf8 &descriptor) const {
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
    return *(MjvmMethodInfo *)0;
}

MjvmMethodInfo &MjvmClassLoader::getMethodInfo(MjvmConstNameAndType &nameAndType) const {
    return getMethodInfo(nameAndType.name, nameAndType.descriptor);
}

MjvmMethodInfo &MjvmClassLoader::getMainMethodInfo(void) const {
    static const uint32_t nameAndType[] = {
        (uint32_t)"\x04\x00\xA5\x01""main",                     /* method name */
        (uint32_t)"\x16\x00\xA2\x07""([Ljava/lang/String;)V",   /* method type */
    };
    return getMethodInfo(*(MjvmConstNameAndType *)nameAndType);
}

MjvmMethodInfo &MjvmClassLoader::getStaticConstructor(void) const {
    static const uint32_t nameAndType[] = {
        (uint32_t)"\x08\x00\xFD\x02""<clinit>",                 /* method name */
        (uint32_t)"\x03\x00\xA7\x00""()V",                      /* method type */
    };
    return getMethodInfo(*(MjvmConstNameAndType *)nameAndType);
}

MjvmClassLoader::~MjvmClassLoader(void) {
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
                case CONST_CLASS:
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
            fields[i].~MjvmFieldInfo();
        Mjvm::free(fields);
    }
    if(methodsCount) {
        for(uint32_t i = 0; i < methodsCount; i++)
            methods[i].~MjvmMethodInfo();
        Mjvm::free(methods);
    }
    for(MjvmAttribute *node = attributes; node != 0;) {
        MjvmAttribute *next = node->next;
        node->~MjvmAttribute();
        Mjvm::free(node);
        node = next;
    }
    Mjvm::free(attributes);
}
