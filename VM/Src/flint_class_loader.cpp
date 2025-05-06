
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
    FlintJavaClass *constClass;
} ConstClassValue;

static void *ClassLoader_Open(const char *fileName, uint32_t length) {
    char buff[FILE_NAME_BUFF_SIZE];
    memcpy(buff, fileName, length);
    memcpy(&buff[length], ".class", sizeof(".class"));

    void *file = FlintAPI::IO::fopen(buff, FLINT_FILE_READ);

    return file;
}

static FlintError ClassLoader_Read(void *file, void *buff, uint32_t size) {
    uint32_t temp;
    FlintFileResult ret = FlintAPI::IO::fread(file, buff, size, &temp);
    if((ret != FILE_RESULT_OK) || (temp != size))
        return ERR_CLASS_FILE_ERROR;
    return ERR_OK;
}

static FlintError ClassLoader_ReadUInt8(void *file, uint8_t &value) {
    return ClassLoader_Read(file, &value, sizeof(uint8_t));
}

static FlintError ClassLoader_ReadUInt16(void *file, uint16_t &value) {
    RETURN_IF_ERR(ClassLoader_Read(file, &value, sizeof(uint16_t)));
    value = SWAP16(value);
    return ERR_OK;
}

static FlintError ClassLoader_ReadUInt32(void *file, uint32_t &value) {
    RETURN_IF_ERR(ClassLoader_Read(file, &value, sizeof(uint32_t)));
    value = SWAP32(value);
    return ERR_OK;
}

static FlintError ClassLoader_ReadUInt64(void *file, uint64_t &value) {
    RETURN_IF_ERR(ClassLoader_Read(file, &value, sizeof(uint64_t)));
    value = SWAP64(value);
    return ERR_OK;
}

static FlintError ClassLoader_Offset(void *file, int32_t offset) {
    if(FlintAPI::IO::fseek(file, FlintAPI::IO::ftell(file) + offset) != FILE_RESULT_OK)
        return ERR_CLASS_FILE_ERROR;
    return ERR_OK;
}

static FlintError dumpAttribute(void *file) {
    RETURN_IF_ERR(ClassLoader_Offset(file, 2)); /* nameIndex */
    uint32_t length;
    RETURN_IF_ERR(ClassLoader_ReadUInt32(file, length));
    RETURN_IF_ERR(ClassLoader_Offset(file, length));
    return ERR_OK;
}

static FlintAttributeType parseAttributeType(const FlintConstUtf8 &name) {
    switch(name.length) {
        case 4:
            if(strncmp(name.text, "Code", name.length) == 0)
                return ATTRIBUTE_CODE;
            break;
        case 13:
            if(strncmp(name.text, "ConstantValue", name.length) == 0)
                return ATTRIBUTE_CONSTANT_VALUE;
            break;
        case 16:
            if(strncmp(name.text, "BootstrapMethods", name.length) == 0)
                return ATTRIBUTE_BOOTSTRAP_METHODS;
            break;
        default:
            break;
    }
    return ATTRIBUTE_UNKNOW;
}

FlintClassLoader::FlintClassLoader(Flint &flint, const char *fileName, uint16_t length) : thisClass(NULL), superClass(NULL), flint(flint) {
    staticCtorInfo = 0;
    poolCount = 0;
    interfacesCount = 0;
    fieldsCount = 0;
    methodsCount = 0;

    load(fileName, length);
}

FlintError FlintClassLoader::load(const char *fileName, uint16_t length) {
    void *file = ClassLoader_Open(fileName, length);
    if(file == NULL)
        return ERR_CLASS_NOT_FOUND;
    FlintError err = load(file);
    if(FlintAPI::IO::fclose(file) != FILE_RESULT_OK)
        return ERR_CLASS_LOAD_FAIL;
    return err;
}

FlintError FlintClassLoader::load(void *file) {
    char buff[FILE_NAME_BUFF_SIZE];
    char *utf8Buff = buff;
    uint16_t utf8Length = sizeof(buff);
    /* RETURN_IF_ERR(ClassLoader_ReadUInt32(file, magic)); */         /* magic = */
    /* RETURN_IF_ERR(ClassLoader_ReadUInt16(file, minorVersion)); */  /* minorVersion = */
    /* RETURN_IF_ERR(ClassLoader_ReadUInt16(file, majorVersion)); */  /* majorVersion = */
    RETURN_IF_ERR(ClassLoader_Offset(file, 8));
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, poolCount));
    poolCount--;
    poolTable = (FlintConstPool *)Flint::malloc(poolCount * sizeof(FlintConstPool));
    for(uint32_t i = 0; i < poolCount; i++) {
        uint8_t tag;
        RETURN_IF_ERR(ClassLoader_ReadUInt8(file, tag));
        *(FlintConstPoolTag *)&poolTable[i].tag = (FlintConstPoolTag)tag;
        switch(tag) {
            case CONST_UTF8: {
                uint16_t length;
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, length));
                if(length > utf8Length) {
                    if(utf8Buff == buff)
                        utf8Buff = (char *)Flint::malloc(length);
                    else
                        utf8Buff = (char *)Flint::realloc(utf8Buff, length);
                    utf8Length = length;
                }
                RETURN_IF_ERR(ClassLoader_Read(file, utf8Buff, length));
                *(uint32_t *)&poolTable[i].value = (uint32_t)&flint.getConstUtf8(utf8Buff, length);
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
                RETURN_IF_ERR(ClassLoader_ReadUInt32(file, *(uint32_t *)&poolTable[i].value));
                break;
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, ((uint16_t *)&poolTable[i].value)[0]));
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, ((uint16_t *)&poolTable[i].value)[1]));
                break;
            case CONST_LONG:
            case CONST_DOUBLE: {
                uint64_t value;
                RETURN_IF_ERR(ClassLoader_ReadUInt64(file, value));
                *(uint32_t *)&poolTable[i + 0].value = (uint32_t)value;
                *(uint32_t *)&poolTable[i + 1].value = (uint32_t)(value >> 32);
                *(FlintConstPoolTag *)&poolTable[i + 1].tag = CONST_UNKOWN;
                i++;
                break;
            }
            case CONST_CLASS:
            case CONST_STRING:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
            case CONST_METHOD_TYPE: {
                uint16_t tmp;
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, tmp));
                *(uint32_t *)&poolTable[i].value = tmp;
                break;
            }
            case CONST_METHOD_HANDLE:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                RETURN_IF_ERR(ClassLoader_ReadUInt8(file, ((uint8_t *)&poolTable[i].value)[0]));
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, ((uint16_t *)&poolTable[i].value)[1]));
                break;
            default:
                return ERR_CLASS_FILE_ERROR;
        }
    }
    if(utf8Buff != buff)
        Flint::free(utf8Buff);

    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, accessFlags));

    uint16_t clsIndex;
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, clsIndex));
    *(FlintConstUtf8 **)&thisClass = &getConstUtf8Class(clsIndex);

    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, clsIndex));
    *(FlintConstUtf8 **)&superClass = (clsIndex != 0) ? &getConstUtf8Class(clsIndex) : 0;

    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, interfacesCount));
    if(interfacesCount) {
        interfaces = (uint16_t *)Flint::malloc(interfacesCount * sizeof(uint16_t));
        for(uint32_t i = 0; i < interfacesCount; i++)
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, interfaces[i]));
    }

    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, fieldsCount));
    if(fieldsCount) {
        uint32_t loadedCount = 0;
        fields = (FlintFieldInfo *)Flint::malloc(fieldsCount * sizeof(FlintFieldInfo));
        for(uint16_t i = 0; i < fieldsCount; i++) {
            uint16_t flag, fieldsNameIndex, fieldsDescriptorIndex, fieldsAttributesCount;
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, flag));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, fieldsNameIndex));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, fieldsDescriptorIndex));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, fieldsAttributesCount));
            while(fieldsAttributesCount--) {
                uint16_t nameIndex;
                uint32_t length;
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, nameIndex));
                RETURN_IF_ERR(ClassLoader_ReadUInt32(file, length));
                FlintAttributeType type = parseAttributeType(getConstUtf8(nameIndex));
                if(
                    type == ATTRIBUTE_CONSTANT_VALUE &&
                    (flag & (FIELD_STATIC | FIELD_FINAL)) == (FIELD_STATIC | FIELD_FINAL)
                ) {
                    flag = (flag | FIELD_UNLOAD);
                }
                RETURN_IF_ERR(ClassLoader_Offset(file, length));
            }
            if(!(flag & FIELD_UNLOAD)) {
                new (&fields[loadedCount])FlintFieldInfo(*this, (FlintFieldAccessFlag)flag, fieldsNameIndex, fieldsDescriptorIndex);
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

    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, methodsCount));
    if(methodsCount) {
        methods = (FlintMethodInfo *)Flint::malloc(methodsCount * sizeof(FlintMethodInfo));
        for(uint16_t i = 0; i < methodsCount; i++) {
            uint16_t flag, methodNameIndex, methodDescriptorIndex, methodAttributesCount;
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, flag));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, methodNameIndex));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, methodDescriptorIndex));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, methodAttributesCount));
            if(!(flag & METHOD_NATIVE)) {
                flag = (flag | METHOD_UNLOADED);
                if(&getConstUtf8(methodNameIndex) == &staticConstructorName)
                    flag = (flag | METHOD_SYNCHRONIZED);
            }
            new (&methods[i])FlintMethodInfo(*this, (FlintMethodAccessFlag)flag, methodNameIndex, methodDescriptorIndex);
            while(methodAttributesCount--) {
                uint16_t nameIndex;
                uint32_t length;
                RETURN_IF_ERR(ClassLoader_ReadUInt16(file, nameIndex));
                RETURN_IF_ERR(ClassLoader_ReadUInt32(file, length));
                FlintAttributeType type = parseAttributeType(getConstUtf8(nameIndex));
                if(type == ATTRIBUTE_CODE && !(flag & METHOD_NATIVE))
                    methods[i].code = (uint8_t *)FlintAPI::IO::ftell(file);
                RETURN_IF_ERR(ClassLoader_Offset(file, length));
            }
        }
    }
    /*
    uint16_t attributesCount;
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, attributesCount));
    while(attributesCount--)
        FlintAttribute *attr = readAttribute(file);
    */
    return ERR_OK;
}

FlintError FlintClassLoader::readAttributeCode(void *file, FlintMethodInfo &method) {
    uint16_t maxStack, maxLocals;
    uint32_t codeLength;
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, maxStack));
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, maxLocals));
    RETURN_IF_ERR(ClassLoader_ReadUInt32(file, codeLength));
    uint32_t codePos = FlintAPI::IO::ftell(file);

    RETURN_IF_ERR(ClassLoader_Offset(file, codeLength));
    uint16_t exceptionTableLength;
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, exceptionTableLength));

    uint32_t codeAttrSize = sizeof(FlintCodeAttribute) + exceptionTableLength * sizeof(FlintExceptionTable) + codeLength + 1;
    FlintCodeAttribute *codeAttr = (FlintCodeAttribute *)Flint::malloc(codeAttrSize);
    codeAttr->maxStack = maxStack;
    codeAttr->maxLocals = maxLocals;
    codeAttr->codeLength = codeLength;
    codeAttr->exceptionLength = exceptionTableLength;

    if(exceptionTableLength) {
        FlintExceptionTable *exceptionTable = (FlintExceptionTable *)codeAttr->data;
        for(uint16_t i = 0; i < exceptionTableLength; i++) {
            uint16_t startPc, endPc, handlerPc, catchType;
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, startPc));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, endPc));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, handlerPc));
            RETURN_IF_ERR(ClassLoader_ReadUInt16(file, catchType));
            new (&exceptionTable[i])FlintExceptionTable(startPc, endPc, handlerPc, catchType);
        }
    }

    uint16_t attrbutesCount;
    RETURN_IF_ERR(ClassLoader_ReadUInt16(file, attrbutesCount));
    while(attrbutesCount--)
        RETURN_IF_ERR(dumpAttribute(file));

    FlintAPI::IO::fseek(file, codePos);
    uint8_t *code = (uint8_t *)&((FlintExceptionTable *)codeAttr->data)[exceptionTableLength];
    RETURN_IF_ERR(ClassLoader_Read(file, code, codeLength));
    code[codeLength] = OP_EXIT;

    method.code = (uint8_t *)codeAttr;

    return ERR_OK;
}

/*
uint32_t FlintClassLoader::getMagic(void) const {
    return magic;
}
*/

/*
uint16_t FlintClassLoader::getMinorVersion(void) const {
    return minorVersion;
}
*/

/*
uint16_t FlintClassLoader::getMajorversion(void) const {
    return majorVersion;
}
*/

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

FlintError FlintClassLoader::getConstClass(uint16_t poolIndex, FlintJavaClass *&cls) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_CLASS) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                FlintConstUtf8 &constUtf8Class = getConstUtf8(poolTable[poolIndex].value);
                ConstClassValue *constClassValue = (ConstClassValue *)Flint::malloc(sizeof(ConstClassValue));
                if(constClassValue == NULL) {
                    Flint::unlock();
                    return ERR_OUT_OF_MEMORY;
                }
                constClassValue->constUtf8Class = &constUtf8Class;
                FlintError err = flint.getConstClass(constUtf8Class.text, constUtf8Class.length, constClassValue->constClass);
                if(err != ERR_OK) {
                    Flint::unlock();
                    return err;
                }
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)constClassValue;
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_CLASS;
            }
            Flint::unlock();
        }
        cls = ((ConstClassValue *)poolTable[poolIndex].value)->constClass;
        return ERR_OK;
    }
    throw ERR_CLASS_FILE_ERROR;
}

FlintError FlintClassLoader::getConstClass(FlintConstPool &constPool, FlintJavaClass *&cls) {
    if((constPool.tag & 0x7F) == CONST_CLASS) {
        if(constPool.tag & 0x80) {
            Flint::lock();
            if(constPool.tag & 0x80) {
                FlintConstUtf8 &constUtf8Class = getConstUtf8(constPool.value);
                ConstClassValue *constClassValue = (ConstClassValue *)Flint::malloc(sizeof(ConstClassValue));
                if(constClassValue == NULL) {
                    Flint::unlock();
                    return ERR_OUT_OF_MEMORY;
                }
                constClassValue->constUtf8Class = &constUtf8Class;
                FlintError err = flint.getConstClass(constUtf8Class.text, constUtf8Class.length, constClassValue->constClass);
                if(err != ERR_OK) {
                    Flint::unlock();
                    return err;
                }
                *(uint32_t *)&constPool.value = (uint32_t)constClassValue;
                *(FlintConstPoolTag *)&constPool.tag = CONST_CLASS;
            }
            Flint::unlock();
        }
        cls = ((ConstClassValue *)constPool.value)->constClass;
        return ERR_OK;
    }
    return ERR_CLASS_FILE_ERROR;
}

FlintError FlintClassLoader::getConstString(uint16_t poolIndex, FlintJavaString *&str) {
    poolIndex--;
    if(poolIndex < poolCount && (poolTable[poolIndex].tag & 0x7F) == CONST_STRING) {
        if(poolTable[poolIndex].tag & 0x80) {
            Flint::lock();
            if(poolTable[poolIndex].tag & 0x80) {
                FlintConstUtf8 &utf8Str = getConstUtf8(poolTable[poolIndex].value);
                FlintError err = flint.getConstString(utf8Str, str);
                if(err != ERR_OK) {
                    Flint::unlock();
                    return err;
                }
                *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)str;
                *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_STRING;
            }
            Flint::unlock();
        }
        str = (FlintJavaString *)poolTable[poolIndex].value;
        return ERR_OK;
    }
    return ERR_CLASS_FILE_ERROR;
}

FlintError FlintClassLoader::getConstString(FlintConstPool &constPool, FlintJavaString *&str) {
    if((constPool.tag & 0x7F) == CONST_STRING) {
        if(constPool.tag & 0x80) {
            Flint::lock();
            if(constPool.tag & 0x80) {
                FlintConstUtf8 &utf8Str = getConstUtf8(constPool.value);
                FlintError err = flint.getConstString(utf8Str, str);
                if(err != ERR_OK) {
                    Flint::unlock();
                    return err;
                }
                *(uint32_t *)&constPool.value = (uint32_t)str;
                *(FlintConstPoolTag *)&constPool.tag = CONST_STRING;
            }
            Flint::unlock();
        }
        str = (FlintJavaString *)constPool.value;
        return ERR_OK;
    }
    return ERR_CLASS_FILE_ERROR;
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
                try {
                    uint16_t nameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                    uint16_t descriptorIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                    *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstNameAndType));
                    new ((FlintConstNameAndType *)poolTable[poolIndex].value)FlintConstNameAndType(getConstUtf8(nameIndex), getConstUtf8(descriptorIndex));
                    *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_NAME_AND_TYPE;
                }
                catch(...) {
                    Flint::unlock();
                    throw;
                }
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
                try {
                    uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                    uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                    *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstField));
                    new ((FlintConstField *)poolTable[poolIndex].value)FlintConstField(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
                    *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_FIELD;
                }
                catch(...) {
                    Flint::unlock();
                    throw;
                }
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
                try {
                    uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                    uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                    *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstMethod));
                    new ((FlintConstMethod *)poolTable[poolIndex].value)FlintConstMethod(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
                    *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_METHOD;
                }
                catch(...) {
                    Flint::unlock();
                    throw;
                }
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
                try {
                    uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
                    uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
                    *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)Flint::malloc(sizeof(FlintConstInterfaceMethod));
                    new ((FlintConstInterfaceMethod *)poolTable[poolIndex].value)FlintConstInterfaceMethod(getConstUtf8Class(classNameIndex), getConstNameAndType(nameAndTypeIndex));
                    *(FlintConstPoolTag *)&poolTable[poolIndex].tag = CONST_INTERFACE_METHOD;
                }
                catch(...) {
                    Flint::unlock();
                    throw;
                }
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

FlintFieldInfo *FlintClassLoader::getFieldInfo(uint8_t fieldIndex) const {
    return &fields[fieldIndex];
}

FlintFieldInfo *FlintClassLoader::getFieldInfo(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) const {
    uint32_t nameHash = CONST_UTF8_HASH(name);
    uint32_t descriptorHash = CONST_UTF8_HASH(descriptor);
    for(uint16_t i = 0; i < fieldsCount; i++) {
        FlintConstUtf8 &fieldName = fields[i].getName();
        FlintConstUtf8 &fieldDesc = fields[i].getDescriptor();
        if(nameHash == CONST_UTF8_HASH(fieldName) && descriptorHash == CONST_UTF8_HASH(fieldDesc)) {
            if(&name == &fields[i].getName() && &descriptor == &fieldDesc)
                return &fields[i];
            else if(
                strncmp(name.text, fieldName.text, name.length) == 0 &&
                strncmp(descriptor.text, fieldDesc.text, descriptor.length) == 0
            ) {
                return &fields[i];
            }
        }
    }
    return NULL;
}

FlintFieldInfo *FlintClassLoader::getFieldInfo(FlintConstNameAndType &nameAndType) const {
    return getFieldInfo(nameAndType.name, nameAndType.descriptor);
}

uint16_t FlintClassLoader::getMethodsCount(void) const {
    return methodsCount;
}

FlintMethodInfo *FlintClassLoader::getMethodInfoWithUnload(uint8_t methodIndex) {
    return &methods[methodIndex];
}

FlintError FlintClassLoader::getMethodInfo(uint8_t methodIndex, FlintMethodInfo *&methodInfo) {
    FlintMethodInfo &method = methods[methodIndex];
    if(method.accessFlag & METHOD_UNLOADED) {
        Flint::lock();
        if(method.accessFlag & METHOD_UNLOADED) {
            void *file = ClassLoader_Open(thisClass->text, thisClass->length);
            if(file == NULL) {
                Flint::unlock();
                return ERR_CLASS_LOAD_FAIL;
            }
            if(FlintAPI::IO::fseek(file, (uint32_t)method.code) != FILE_RESULT_OK) {
                Flint::unlock();
                FlintAPI::IO::fclose(file);
                return ERR_CLASS_LOAD_FAIL;
            }
            readAttributeCode(file, method);
            if(FlintAPI::IO::fclose(file) != FILE_RESULT_OK) {
                Flint::unlock();
                return ERR_CLASS_LOAD_FAIL;
            }
            method.accessFlag = (FlintMethodAccessFlag)(method.accessFlag & ~METHOD_UNLOADED);
        }
        Flint::unlock();
    }
    methodInfo = &method;
    return ERR_OK;
}

FlintError FlintClassLoader::getMethodInfo(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor, FlintMethodInfo *&methodInfo) {
    uint32_t nameHash = CONST_UTF8_HASH(name);
    uint32_t descriptorHash = CONST_UTF8_HASH(descriptor);
    for(uint16_t i = 0; i < methodsCount; i++) {
        FlintConstUtf8 &methodName = methods[i].getName();
        FlintConstUtf8 &methodDesc = methods[i].getDescriptor();
        if(nameHash == CONST_UTF8_HASH(methodName) && descriptorHash == CONST_UTF8_HASH(methodDesc)) {
            if(&name == &methodName && &descriptor == &methodDesc)
                return getMethodInfo(i, methodInfo);
            else if(
                strncmp(name.text, methodName.text, name.length) == 0 &&
                strncmp(descriptor.text, methodDesc.text, descriptor.length) == 0
            ) {
                return getMethodInfo(i, methodInfo);
            }
        }
    }
    return ERR_METHOD_NOT_FOUND;
}

FlintError FlintClassLoader::getMethodInfo(FlintConstNameAndType &nameAndType, FlintMethodInfo *&methodInfo) {
    return getMethodInfo(nameAndType.name, nameAndType.descriptor, methodInfo);
}

FlintMethodInfo *FlintClassLoader::getMethodInfoWithUnload(const FlintConstUtf8 &name, const FlintConstUtf8 &descriptor) {
    uint32_t nameHash = CONST_UTF8_HASH(name);
    uint32_t descriptorHash = CONST_UTF8_HASH(descriptor);
    for(uint16_t i = 0; i < methodsCount; i++) {
        FlintConstUtf8 &methodName = methods[i].getName();
        FlintConstUtf8 &methodDesc = methods[i].getDescriptor();
        if(nameHash == CONST_UTF8_HASH(methodName) && descriptorHash == CONST_UTF8_HASH(methodDesc)) {
            if(&name == &methodName && &descriptor == &methodDesc)
                return getMethodInfoWithUnload(i);
            else if(
                strncmp(name.text, methodName.text, name.length) == 0 &&
                strncmp(descriptor.text, methodDesc.text, descriptor.length) == 0
            ) {
                return getMethodInfoWithUnload(i);
            }
        }
    }
    return NULL;
}

FlintError FlintClassLoader::getMainMethodInfo(FlintMethodInfo *&methodInfo) {
    static const uint32_t nameAndType[] = {
        (uint32_t)"\x04\x00\x1D\x15""main",                     /* method name */
        (uint32_t)"\x16\x00\x03\x78""([Ljava/lang/String;)V",   /* method type */
    };
    return getMethodInfo(*(FlintConstNameAndType *)nameAndType, methodInfo);
}

FlintError FlintClassLoader::getStaticCtor(FlintMethodInfo *&methodInfo) {
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(&staticConstructorName == &methods[i].getName())
            return getMethodInfo(i, methodInfo);
    }
    return ERR_METHOD_NOT_FOUND;
}

bool FlintClassLoader::hasStaticCtor(void) {
    if(!staticCtorInfo) {
        bool isFound = false;
        for(uint16_t i = 0; i < methodsCount; i++) {
            if(&staticConstructorName == &methods[i].getName()) {
                isFound = true;
                break;
            }
        }
        staticCtorInfo = isFound ? 0x81 : 0x80;
    }
    return staticCtorInfo & 0x01;
}

FlintClassLoader::~FlintClassLoader(void) {
    if(poolCount) {
        for(uint32_t i = 0; i < poolCount; i++) {
            switch (poolTable[i].tag) {
                case CONST_UTF8:
                    break;
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
}
