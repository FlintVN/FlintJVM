
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_opcodes.h"
#include "flint_common.h"
#include "flint_default_conf.h"
#include "flint_class_loader.h"

#define FLAG_HAS_STATIC_FIELD   0x01
#define FLAG_HAS_CLINIT         0x02
#define FLAG_STATIC_INIT        0x08

typedef struct {
    ConstPoolTag tag;
    uint16_t clsNameIndex;
    JClass *cls;
} ConstClass;

static int16_t Append(char *buff, int32_t index, const char *str, uint16_t len = 0xFFFF) {
    if(index >= FILE_NAME_BUFF_SIZE) return -1;
    while(*str && len--) {
        buff[index++] = *str++;
        if(index >= FILE_NAME_BUFF_SIZE) return -1;
    }
    buff[index] = 0;
    return index;
}

static int16_t CombinePath(char *buff, const char *folder, uint16_t folderLen, const char *clsName, uint16_t clsLen) {
    int32_t index = 0;
    if(index = Append(buff, index, folder, folderLen); index == -1) return -1;
    char separatorChar = GetPathSeparatorChar();
    if(buff[index - 1] != separatorChar) {
        if(index >= FILE_NAME_BUFF_SIZE) return -1;
        buff[index++] = separatorChar;
    }
    if(index = Append(buff, index, clsName, clsLen); index == -1) return -1;
    return Append(buff, index, ".class");
}

static FlintAPI::IO::FileHandle FOpen(const char *fileName, uint16_t length = 0xFFFF) {
    int16_t index;
    char buff[FILE_NAME_BUFF_SIZE];
    if(index = ResolvePath(fileName, length, buff, sizeof(buff)); index == -1) return NULL;
    if(index = Append(buff, index, ".class"); index == -1) return NULL;
    if(FlintAPI::IO::finfo(buff, NULL) == FlintAPI::IO::FILE_RESULT_OK)
        return FlintAPI::IO::fopen(buff, FlintAPI::IO::FILE_MODE_READ);

    const char *jdks = Flint::getClassPaths();
    if(jdks) {
        while(1) {
            uint32_t len = 0;
            while(jdks[len] != 0 && jdks[len] != ';') len++;
            if(CombinePath(buff, jdks, len, fileName, length) == -1) return NULL;
            if(FlintAPI::IO::finfo(buff, NULL) == FlintAPI::IO::FILE_RESULT_OK)
                return FlintAPI::IO::fopen(buff, FlintAPI::IO::FILE_MODE_READ);
            if(jdks[len] == 0) return NULL;
            jdks += len + 1;
        }
    }
    return NULL;
}

static bool FRead(FExec *ctx, FlintAPI::IO::FileHandle file, void *buff, uint32_t size) {
    uint32_t temp;
    FlintAPI::IO::FileResult ret = FlintAPI::IO::fread(file, buff, size, &temp);
    if((ret != FlintAPI::IO::FILE_RESULT_OK) || (temp != size)) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "FlintAPI::IO::fread failed");
        return false;
    }
    return true;
}

static bool FReadUInt8(FExec *ctx, FlintAPI::IO::FileHandle file, uint8_t &value) {
    return FRead(ctx, file, &value, sizeof(uint8_t));
}

static bool FReadUInt16(FExec *ctx, FlintAPI::IO::FileHandle file, uint16_t &value) {
    if(!FRead(ctx, file, &value, sizeof(uint16_t))) return false;
    value = Swap16(value);
    return true;
}

static bool FReadUInt32(FExec *ctx, FlintAPI::IO::FileHandle file, uint32_t &value) {
    if(!FRead(ctx, file, &value, sizeof(uint32_t))) return false;
    value = Swap32(value);
    return true;
}

static bool FReadUInt64(FExec *ctx, FlintAPI::IO::FileHandle file, uint64_t &value) {
    if(!FRead(ctx, file, &value, sizeof(uint64_t))) return false;
    value = Swap64(value);
    return true;
}

static bool Fseek(FExec *ctx, FlintAPI::IO::FileHandle file, int32_t offset) {
    if(FlintAPI::IO::fseek(file, offset) != FlintAPI::IO::FILE_RESULT_OK) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "FlintAPI::IO::fseek failed");
        return false;
    }
    return true;
}

static bool FOffset(FExec *ctx, FlintAPI::IO::FileHandle file, int32_t offset) {
    return Fseek(ctx, file, FlintAPI::IO::ftell(file) + offset);
}

static bool FClose(FExec *ctx, FlintAPI::IO::FileHandle handle) {
    if(FlintAPI::IO::fclose(handle) != FlintAPI::IO::FILE_RESULT_OK) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "FlintAPI::IO::fclose failed");
        return false;
    }
    return true;
}

static bool dumpAttribute(FExec *ctx, void *file) {
    if(!FOffset(ctx, file, 2)) return false; /* nameIndex */
    uint32_t length;
    if(!FReadUInt32(ctx, file, length)) return false;
    if(!FOffset(ctx, file, length)) return false;
    return true;
}

ClassLoader::ClassLoader(void) : DictNode() {
    loaderFlags = 0;
    poolCount = 0;
    thisClass = 0;
    superClass = 0;
    interfacesCount = 0;
    fieldsCount = 0;
    methodsCount = 0;
    nestHost = 0;
    nestMembersCount = 0;
    hash = 0;
    monitorOwnId = 0;
    monitorCount = 0;
    poolTable = NULL;
    interfaces = NULL;
    fields = NULL;
    methods = NULL;
    nestMembers = NULL;
    staticFields = NULL;
}

uint32_t ClassLoader::getHashKey(void) const {
    return hash;
}

int32_t ClassLoader::compareKey(const char *key, uint16_t length) const {
    return (length > 0) ? strncmp(this->getName(), key, length) : strcmp(this->getName(), key);
}

int32_t ClassLoader::compareKey(DictNode *other) const {
    return strcmp(this->getName(), ((ClassLoader *)other)->getName());
}

bool ClassLoader::load(FExec *ctx, FlintAPI::IO::FileHandle file) {
    char buff[FILE_NAME_BUFF_SIZE];
    char *utf8Buff = buff;
    uint16_t utf8Length = sizeof(buff);
    /* if(!FReadUInt32(file, magic)) return false; */         /* magic = */
    /* if(!FReadUInt16(file, minorVersion)) return false; */  /* minorVersion = */
    /* if(!FReadUInt16(file, majorVersion)) return false; */  /* majorVersion = */
    if(!FOffset(ctx, file, 8)) return false;
    if(!FReadUInt16(ctx, file, poolCount)) return false;
    poolCount--;
    poolTable = (ConstPool *)Flint::malloc(ctx, poolCount * sizeof(ConstPool));
    if(poolTable == NULL) return false;
    for(uint32_t i = 0; i < poolCount; i++) {
        uint8_t tag;
        if(!FReadUInt8(ctx, file, tag)) return false;
        *(ConstPoolTag *)&poolTable[i].tag = (ConstPoolTag)tag;
        switch(tag) {
            case CONST_UTF8: {
                uint16_t length;
                if(!FReadUInt16(ctx, file, length)) return false;
                if(length > utf8Length) {
                    utf8Buff = (char *)((utf8Buff == buff) ? Flint::malloc(ctx, length) : Flint::realloc(ctx, utf8Buff, length));
                    if(utf8Buff == NULL) return false;
                    utf8Length = length;
                }
                if(!FRead(ctx, file, utf8Buff, length)) return false;
                utf8Buff[length] = 0;
                const char *utf8 = Flint::getUtf8(ctx, utf8Buff);
                if(utf8 == NULL) return false;
                *(uint32_t *)&poolTable[i].value = (uint32_t)utf8;
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
                if(!FReadUInt32(ctx, file, *(uint32_t *)&poolTable[i].value)) return false;
                break;
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                if(!FReadUInt16(ctx, file, ((uint16_t *)&poolTable[i].value)[0])) return false;
                if(!FReadUInt16(ctx, file, ((uint16_t *)&poolTable[i].value)[1])) return false;
                break;
            case CONST_LONG:
            case CONST_DOUBLE: {
                uint64_t value;
                if(!FReadUInt64(ctx, file, value)) return false;
                *(uint32_t *)&poolTable[i + 0].value = (uint32_t)value;
                *(uint32_t *)&poolTable[i + 1].value = (uint32_t)(value >> 32);
                *(ConstPoolTag *)&poolTable[i + 1].tag = CONST_UNKOWN;
                i++;
                break;
            }
            case CONST_CLASS: {
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                ((ConstClass *)&poolTable[i])->cls = NULL;
                if(!FReadUInt16(ctx, file, ((ConstClass *)&poolTable[i])->clsNameIndex)) return false;
                break;
            }
            case CONST_STRING:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
            case CONST_METHOD_TYPE: {
                uint16_t tmp;
                if(!FReadUInt16(ctx, file, tmp)) return false;
                *(uint32_t *)&poolTable[i].value = tmp;
                break;
            }
            case CONST_METHOD_HANDLE:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                if(!FReadUInt8(ctx, file, ((uint8_t *)&poolTable[i].value)[0])) return false;
                if(!FReadUInt16(ctx, file, ((uint16_t *)&poolTable[i].value)[1])) return false;
                break;
            default: {
                if(ctx != NULL) {
                    JClass *excpCls = Flint::findClass(ctx, "java/lang/ClassFormatError");
                    ctx->throwNew(excpCls, "Constant pool tag value (%u) is invalid", tag);
                }
                return false;
            }
        }
    }
    if(utf8Buff != buff) Flint::free(utf8Buff);

    if(!FReadUInt16(ctx, file, accessFlags)) return false;

    if(!FReadUInt16(ctx, file, thisClass)) return false;
    hash = Hash(getName());

    if(!FReadUInt16(ctx, file, superClass)) return false;

    if(!FReadUInt16(ctx, file, interfacesCount)) return false;
    if(interfacesCount) {
        interfaces = (uint16_t *)Flint::malloc(ctx, interfacesCount * sizeof(uint16_t));
        if(interfaces == NULL) return false;
        for(uint32_t i = 0; i < interfacesCount; i++)
            if(!FReadUInt16(ctx, file, interfaces[i])) return false;
    }

    if(!FReadUInt16(ctx, file, fieldsCount)) return false;
    if(fieldsCount) {
        uint32_t loadedCount = 0;
        fields = (FieldInfo *)Flint::malloc(ctx, fieldsCount * sizeof(FieldInfo));
        if(fields == NULL) return false;
        for(uint16_t i = 0; i < fieldsCount; i++) {
            uint16_t flag, fieldsNameIndex, fieldsDescIndex, fieldsAttributesCount;
            if(!FReadUInt16(ctx, file, flag)) return false;
            if(!FReadUInt16(ctx, file, fieldsNameIndex)) return false;
            if(!FReadUInt16(ctx, file, fieldsDescIndex)) return false;
            if(!FReadUInt16(ctx, file, fieldsAttributesCount)) return false;
            while(fieldsAttributesCount--) {
                uint16_t attrNameIdx;
                uint32_t length;
                if(!FReadUInt16(ctx, file, attrNameIdx)) return false;
                if(!FReadUInt32(ctx, file, length)) return false;
                if(
                    strcmp(getConstUtf8(attrNameIdx), "ConstantValue") == 0 &&
                    (flag & (FIELD_STATIC | FIELD_FINAL)) == (FIELD_STATIC | FIELD_FINAL)
                ) {
                    flag = (flag | FIELD_UNLOAD);
                }
                if(!FOffset(ctx, file, length)) return false;
            }
            if(!(flag & FIELD_UNLOAD)) {
                const char *fieldName = getConstUtf8(fieldsNameIndex);
                const char *fieldDesc = getConstUtf8(fieldsDescIndex);
                new (&fields[loadedCount])FieldInfo((FieldAccessFlag)flag, fieldName, fieldDesc);
                loadedCount++;
                if(flag & FIELD_STATIC)
                    loaderFlags |= FLAG_HAS_STATIC_FIELD;
            }
        }
        if(loadedCount == 0) {
            Flint::free(fields);
            fields = NULL;
            fieldsCount = 0;
        }
        else if(loadedCount != fieldsCount) {
            fields = (FieldInfo *)Flint::realloc(ctx, fields, loadedCount * sizeof(FieldInfo));
            if(fields == NULL) return false;
            fieldsCount = loadedCount;
        }
    }

    if(!FReadUInt16(ctx, file, methodsCount)) return false;
    if(methodsCount) {
        methods = (MethodInfo *)Flint::malloc(ctx, methodsCount * sizeof(MethodInfo));
        if(methods == NULL) return false;
        for(uint16_t i = 0; i < methodsCount; i++) {
            uint16_t flag, methodNameIndex, methodDescIndex, methodAttributesCount;
            if(!FReadUInt16(ctx, file, flag)) return false;
            if(!FReadUInt16(ctx, file, methodNameIndex)) return false;
            if(!FReadUInt16(ctx, file, methodDescIndex)) return false;
            if(!FReadUInt16(ctx, file, methodAttributesCount)) return false;
            if(!(flag & METHOD_NATIVE)) {
                flag |= METHOD_UNLOADED;
                if((flag & METHOD_STATIC) && strcmp(getConstUtf8(methodNameIndex), "<clinit>") == 0) {
                    flag = (flag | METHOD_CLINIT);
                    loaderFlags |= FLAG_HAS_CLINIT;
                }
                else if(strcmp(getConstUtf8(methodNameIndex), "<init>") == 0)
                    flag = (flag | METHOD_INIT);
            }
            const char *methodName = getConstUtf8(methodNameIndex);
            const char *methodDesc = getConstUtf8(methodDescIndex);
            new (&methods[i])MethodInfo(this, (MethodAccessFlag)flag, methodName, methodDesc);
            while(methodAttributesCount--) {
                uint16_t attrNameIdx;
                uint32_t length;
                if(!FReadUInt16(ctx, file, attrNameIdx)) return false;
                if(!FReadUInt32(ctx, file, length)) return false;
                if(strcmp(getConstUtf8(attrNameIdx), "Code") == 0 && !(flag & METHOD_NATIVE))
                    methods[i].code = (uint8_t *)FlintAPI::IO::ftell(file);
                if(!FOffset(ctx, file, length)) return false;
            }
        }
    }
    uint16_t attributesCount;
    if(!FReadUInt16(ctx, file, attributesCount)) return false;
    while(attributesCount--) {
        uint16_t attrNameIdx;
        uint32_t length;
        if(!FReadUInt16(ctx, file, attrNameIdx)) return false;
        if(!FReadUInt32(ctx, file, length)) return false;
        const char *attrName = getConstUtf8(attrNameIdx);
        if(length == 2 && strcmp(attrName, "NestHost") == 0) {
            if(!FReadUInt16(ctx, file, nestHost)) return false;
        }
        else if(strcmp(attrName, "NestMembers") == 0) {
            if(!FReadUInt16(ctx, file, nestMembersCount)) return false;
            if(nestMembersCount > 0)
                nestMembers = (uint16_t *)Flint::malloc(ctx, nestMembersCount * sizeof(uint16_t));
            for(uint16_t i = 0; i < nestMembersCount; i++)
                if(!FReadUInt16(ctx, file, nestMembers[i])) return false;
        }
        else
            if(!FOffset(ctx, file, length)) return false;
    }
    if(hasStaticCtor() == false && hasStaticField() == false)
        staticInitialized();
    return true;
}

ClassLoader *ClassLoader::load(FExec *ctx, const char *clsName, uint16_t length) {
    FlintAPI::IO::FileHandle file = FOpen(clsName, length);
    if(file == NULL) {
        if(ctx != NULL)
            ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "'%.*s' loading failed", length, clsName);
        return NULL;
    }
    ClassLoader *loader = (ClassLoader *)Flint::malloc(ctx, sizeof(ClassLoader));
    if(loader == NULL) return NULL;
    new (loader)ClassLoader();
    if(loader->load(ctx, file) == false) {
        loader->~ClassLoader();
        Flint::free(loader);
        return NULL;
    }
    if(FClose(ctx, file) == false) {
        if(loader != NULL) {
            loader->~ClassLoader();
            Flint::free(loader);
        }
        return NULL;
    }
    return loader;
}

CodeAttribute *ClassLoader::readAttributeCode(FExec *ctx, void *file) {
    uint16_t maxStack, maxLocals;
    uint32_t codeLength;
    if(!FReadUInt16(ctx, file, maxStack)) return NULL;
    if(!FReadUInt16(ctx, file, maxLocals)) return NULL;
    if(!FReadUInt32(ctx, file, codeLength)) return NULL;
    uint32_t codePos = FlintAPI::IO::ftell(file);

    if(!FOffset(ctx, file, codeLength)) return NULL;
    uint16_t exceptionTableLength;
    if(!FReadUInt16(ctx, file, exceptionTableLength)) return NULL;

    uint32_t codeAttrSize = sizeof(CodeAttribute) + exceptionTableLength * sizeof(ExceptionTable) + codeLength + 1;
    CodeAttribute *codeAttr = (CodeAttribute *)Flint::malloc(ctx, codeAttrSize);
    if(codeAttr == NULL) return NULL;
    codeAttr->maxStack = maxStack;
    codeAttr->maxLocals = maxLocals;
    codeAttr->codeLength = codeLength;
    codeAttr->exceptionLength = exceptionTableLength;

    if(exceptionTableLength) {
        ExceptionTable *exceptionTable = (ExceptionTable *)codeAttr->data;
        for(uint16_t i = 0; i < exceptionTableLength; i++) {
            uint16_t startPc, endPc, handlerPc, catchType;
            if(!FReadUInt16(ctx, file, startPc)) { Flint::free(codeAttr); return NULL; }
            if(!FReadUInt16(ctx, file, endPc)) { Flint::free(codeAttr); return NULL; }
            if(!FReadUInt16(ctx, file, handlerPc)) { Flint::free(codeAttr); return NULL; }
            if(!FReadUInt16(ctx, file, catchType)) { Flint::free(codeAttr); return NULL; }
            new (&exceptionTable[i])ExceptionTable(startPc, endPc, handlerPc, catchType);
        }
    }

    uint16_t attrbutesCount;
    if(!FReadUInt16(ctx, file, attrbutesCount)) { Flint::free(codeAttr); return NULL; }
    while(attrbutesCount--)
        if(!dumpAttribute(ctx, file)) { Flint::free(codeAttr); return NULL; }

    if(!Fseek(ctx, file, codePos)) { Flint::free(codeAttr); return NULL; }
    uint8_t *code = (uint8_t *)&((ExceptionTable *)codeAttr->data)[exceptionTableLength];
    if(!FRead(ctx, file, code, codeLength)) { Flint::free(codeAttr); return NULL; }
    code[codeLength] = OP_EXIT;

    return codeAttr;
}

ConstPoolTag ClassLoader::getConstPoolTag(uint16_t poolIndex) const {
    return (ConstPoolTag)(poolTable[poolIndex - 1].tag & 0x7F);
}

int32_t ClassLoader::getConstInteger(uint16_t poolIndex) const {
    return (int32_t)poolTable[poolIndex - 1].value;
}

float ClassLoader::getConstFloat(uint16_t poolIndex) const {
    return *(float *)&poolTable[poolIndex - 1].value;
}

int64_t ClassLoader::getConstLong(uint16_t poolIndex) const {
    return ((uint64_t)poolTable[poolIndex].value << 32) | poolTable[poolIndex - 1].value;
}

double ClassLoader::getConstDouble(uint16_t poolIndex) const {
    uint64_t ret = ((uint64_t)poolTable[poolIndex].value << 32) | poolTable[poolIndex - 1].value;
    return *(double *)&ret;
}

const char *ClassLoader::getConstUtf8(uint16_t poolIndex) const {
    return (const char *)poolTable[poolIndex - 1].value;
}

const char *ClassLoader::getConstClassName(uint16_t poolIndex) const {
    ConstClass *constCls = (ConstClass *)&poolTable[poolIndex - 1];
    return getConstUtf8(constCls->clsNameIndex);
}

ConstNameAndType *ClassLoader::getConstNameAndType(FExec *ctx, uint16_t poolIndex) {
    poolIndex--;
    if(poolTable[poolIndex].tag & 0x80) {
        Flint::lock();
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t nameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t descIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            ConstNameAndType *tmp = (ConstNameAndType *)Flint::malloc(ctx, sizeof(ConstNameAndType));
            if(tmp == NULL) {
                Flint::unlock();
                return NULL;
            }
            new (tmp)ConstNameAndType(getConstUtf8(nameIndex), getConstUtf8(descIndex));
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)tmp;
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_NAME_AND_TYPE;
        }
        Flint::unlock();
    }
    return (ConstNameAndType *)poolTable[poolIndex].value;
}

ConstField *ClassLoader::getConstField(FExec *ctx, uint16_t poolIndex) {
    poolIndex--;
    if(poolTable[poolIndex].tag & 0x80) {
        Flint::lock();
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            ConstField *tmp = (ConstField *)Flint::malloc(ctx, sizeof(ConstField));
            if(tmp == NULL) {
                Flint::unlock();
                return NULL;
            }
            ConstNameAndType *nameAndType = getConstNameAndType(ctx, nameAndTypeIndex);
            if(nameAndType == NULL) {
                Flint::unlock();
                Flint::free(tmp);
                return NULL;
            }
            new (tmp)ConstField(getConstClassName(classNameIndex), nameAndType);
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)tmp;
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_FIELD;
        }
        Flint::unlock();
    }
    return (ConstField *)poolTable[poolIndex].value;
}

ConstMethod *ClassLoader::getConstMethod(FExec *ctx, uint16_t poolIndex) {
    poolIndex--;
    if(poolTable[poolIndex].tag & 0x80) {
        Flint::lock();
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            ConstMethod *tmp = (ConstMethod *)Flint::malloc(ctx, sizeof(ConstMethod));
            if(tmp == NULL) {
                Flint::unlock();
                return NULL;
            }
            ConstNameAndType *nameAndType = getConstNameAndType(ctx, nameAndTypeIndex);
            if(nameAndType == NULL) {
                Flint::unlock();
                Flint::free(tmp);
                return NULL;
            }
            new (tmp)ConstMethod(getConstClassName(classNameIndex), nameAndType);
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)tmp;
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_METHOD;
        }
        Flint::unlock();
    }
    return (ConstMethod *)poolTable[poolIndex].value;
}

ConstInterfaceMethod *ClassLoader::getConstInterfaceMethod(FExec *ctx, uint16_t poolIndex) {
    poolIndex--;
    if(poolTable[poolIndex].tag & 0x80) {
        Flint::lock();
        if(poolTable[poolIndex].tag & 0x80) {
            uint16_t classNameIndex = ((uint16_t *)&poolTable[poolIndex].value)[0];
            uint16_t nameAndTypeIndex = ((uint16_t *)&poolTable[poolIndex].value)[1];
            ConstInterfaceMethod *tmp = (ConstInterfaceMethod *)Flint::malloc(ctx, sizeof(ConstInterfaceMethod));
            if(tmp == NULL) {
                Flint::unlock();
                return NULL;
            }
            ConstNameAndType *nameAndType = getConstNameAndType(ctx, nameAndTypeIndex);
            if(nameAndType == NULL) {
                Flint::unlock();
                Flint::free(tmp);
                return NULL;
            }
            new (tmp)ConstInterfaceMethod(getConstClassName(classNameIndex), nameAndType);
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)tmp;
            *(ConstPoolTag *)&poolTable[poolIndex].tag = CONST_INTERFACE_METHOD;
        }
        Flint::unlock();
    }
    return (ConstInterfaceMethod *)poolTable[poolIndex].value;
}

JString *ClassLoader::getConstString(FExec *ctx, uint16_t poolIndex) {
    ConstPool *constPool = (ConstPool *)&poolTable[poolIndex - 1];
    if(constPool->tag & 0x80) {
        Flint::lock();
        if(constPool->tag & 0x80) {
            const char *utf8 = getConstUtf8(constPool->value);
            JString *str = Flint::getConstString(ctx, utf8);
            if(str != NULL) {
                *(uint32_t *)&constPool->value = (uint32_t)str;
                *(ConstPoolTag *)&constPool->tag = CONST_STRING;
            }
            Flint::unlock();
            return str;
        }
        Flint::unlock();
    }
    return (JString *)constPool->value;
}

JClass *ClassLoader::getConstClass(FExec *ctx, uint16_t poolIndex) {
    ConstClass *constCls = (ConstClass *)&poolTable[poolIndex - 1];
    if(constCls->tag & 0x80) {
        Flint::lock();
        if(constCls->tag & 0x80) {
            const char *clsName = getConstUtf8(constCls->clsNameIndex);
            JClass *cls = Flint::findClass(ctx, clsName);
            constCls->cls = cls;
            if(cls != NULL)
                constCls->tag = CONST_CLASS;
        }
        Flint::unlock();
    }
    return constCls->cls;
}

ClassAccessFlag ClassLoader::getAccessFlag(void) const {
    return (ClassAccessFlag)accessFlags;
}

const char *ClassLoader::getName(void) const {
    return getConstClassName(thisClass);
}

const char *ClassLoader::getSuperClassName(void) const {
    if(superClass == 0) return NULL;
    return getConstClassName(superClass);
}

JClass *ClassLoader::getThisClass(FExec *ctx) {
    return getConstClass(ctx, thisClass);
}

JClass *ClassLoader::getSuperClass(FExec *ctx) {
    if(superClass == 0) return NULL;
    return getConstClass(ctx, superClass);
}

uint16_t ClassLoader::getInterfacesCount(void) const {
    return interfacesCount;
}

const char *ClassLoader::getInterface(uint16_t interfaceIndex) const {
    return getConstClassName(interfaces[interfaceIndex]);
}

uint16_t ClassLoader::getFieldsCount(void) const {
    return fieldsCount;
}

FieldInfo *ClassLoader::getFieldInfo(uint16_t fieldIndex) const {
    return &fields[fieldIndex];
}

uint16_t ClassLoader::getMethodsCount(void) const {
    return methodsCount;
}

MethodInfo *ClassLoader::getMethodInfo(FExec *ctx, uint8_t methodIndex) {
    MethodInfo *method = &methods[methodIndex];
    if(method->accessFlag & METHOD_UNLOADED) {
        Flint::lock();
        if(method->accessFlag & METHOD_UNLOADED) {
            void *file = FOpen(getName());
            if(file == NULL) {
                Flint::unlock();
                if(ctx != NULL)
                    ctx->throwNew(Flint::findClass(ctx, "java/io/IOException"), "'%s' loading failed", getName());
                return NULL;
            }

            if(!Fseek(ctx, file, (uint32_t)method->code)) { FClose(NULL, file); Flint::unlock(); return NULL; }

            uint8_t *attrCode = (uint8_t *)readAttributeCode(ctx, file);
            if(attrCode == NULL) { FClose(NULL, file); Flint::unlock(); return NULL; }

            if(FClose(ctx, file) == false) { Flint::free(attrCode); Flint::unlock(); return NULL; }

            method->code = attrCode;
            method->accessFlag = (MethodAccessFlag)(method->accessFlag & ~METHOD_UNLOADED);
        }
        Flint::unlock();
    }
    return method;
}

MethodInfo *ClassLoader::getMethodInfo(FExec *ctx, ConstNameAndType *nameAndType) {
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(
            nameAndType->hash == methods[i].hash &&
            strcmp(nameAndType->name, methods[i].name) == 0 &&
            strcmp(nameAndType->desc, methods[i].desc) == 0
        ) {
            return getMethodInfo(ctx, i);
        }
    }
    return NULL;
}

MethodInfo *ClassLoader::getMethodInfo(FExec *ctx, const char *name, const char *desc) {
    uint32_t hash = (Hash(name) & 0xFFFF) | (Hash(desc) << 16);
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(
            hash == methods[i].hash &&
            strcmp(name, methods[i].name) == 0 &&
            strcmp(desc, methods[i].desc) == 0
        ) {
            return getMethodInfo(ctx, i);
        }
    }
    return NULL;
}

MethodInfo *ClassLoader::getMainMethodInfo(FExec *ctx) {
    static constexpr ConstNameAndType mainName("main", "([Ljava/lang/String;)V");
    return getMethodInfo(ctx, (ConstNameAndType *)&mainName);
}

MethodInfo *ClassLoader::getStaticCtor(FExec *ctx) {
    static constexpr ConstNameAndType clinitName("<clinit>", "()V");
    return getMethodInfo(ctx, (ConstNameAndType *)&clinitName);
}

bool ClassLoader::hasStaticField(void) const {
    return (loaderFlags & FLAG_HAS_STATIC_FIELD) ? true : false;
}

bool ClassLoader::hasStaticCtor(void) const {
    return (loaderFlags & FLAG_HAS_CLINIT) ? true : false;
}

JClass *ClassLoader::getNestHost(FExec *ctx) {
    if(nestHost == 0)
        return getThisClass(ctx);
    return getConstClass(ctx, nestHost);
}

uint16_t ClassLoader::getNestMembersCount(void) const {
    return nestMembersCount;
}

JClass *ClassLoader::getNestMember(FExec *ctx, uint16_t index) {
    if(index >= nestMembersCount) return NULL;
    return getConstClass(ctx, nestMembers[index]);
}

static void throwNoSuchFieldError(FExec *ctx, const char *clsName, const char *name) {
    JClass *excpCls = Flint::findClass(ctx, "java/lang/NoSuchFieldError");
    ctx->throwNew(excpCls, "Could not find the field %s.%s", clsName, name);
}

uint16_t ClassLoader::hasStaticObjField(void) const {
    if(staticFields == NULL) return 0;
    return staticFields->hasObjField();
}

FieldValue *ClassLoader::getStaticField(FExec *ctx, ConstField *field) const {
    FieldValue *ret = staticFields->getField(field);
    if(ret == NULL && ctx != NULL)
        throwNoSuchFieldError(ctx, field->className, field->nameAndType->name);
    return ret;
}

FieldValue *ClassLoader::getStaticField(FExec *ctx, const char *name) const {
    FieldValue *ret = staticFields->getField(name);
    if(ret == NULL && ctx != NULL)
        throwNoSuchFieldError(ctx, getName(), name);
    return ret;
}

FieldValue *ClassLoader::getStaticFieldByIndex(uint32_t index) const {
    return staticFields->getFieldByIndex(index);
}

StaticInitStatus ClassLoader::getStaticInitStatus(void) const {
    if(loaderFlags & FLAG_STATIC_INIT)
        return INITIALIZED;
    return (staticFields == NULL) ? UNINITIALIZED : INITIALIZING;
}

void ClassLoader::staticInitialized(void) {
    loaderFlags |= FLAG_STATIC_INIT;
}

bool ClassLoader::initStaticFields(FExec *ctx) {
    staticFields = (FieldsData *)Flint::malloc(ctx, sizeof(FieldsData));
    if(staticFields == NULL) return false;
    new (staticFields)FieldsData();
    return staticFields->init(ctx, this, true);
}

void ClassLoader::clearStaticFields(void) {
    if(staticFields != NULL) {
        staticFields->~FieldsData();
        Flint::free(staticFields);
        staticFields = NULL;
        loaderFlags &= ~FLAG_STATIC_INIT;
    }
}

ClassLoader::~ClassLoader(void) {
    if(poolCount && poolTable) {
        for(uint32_t i = 0; i < poolCount; i++) {
            switch(poolTable[i].tag) {
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
                case CONST_METHOD_HANDLE:
                    Flint::free((void *)poolTable[i].value);
                    break;
                default:
                    break;
            }
        }
        Flint::free(poolTable);
    }
    if(interfacesCount && interfaces)
        Flint::free(interfaces);
    if(fieldsCount && fields)
        Flint::free(fields);
    if(methodsCount && methods) {
        for(uint32_t i = 0; i < methodsCount; i++) {
            if(!(methods[i].accessFlag & (METHOD_NATIVE | METHOD_UNLOADED)) && methods[i].code)
                Flint::free(methods[i].code);
        }
        Flint::free(methods);
    }
    if(nestMembersCount && nestMembers)
        Flint::free(nestMembers);
    clearStaticFields();
}
