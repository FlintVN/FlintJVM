
#include <string.h>
#include "mjvm_heap.h"
#include "mjvm_class_loader.h"

ClassLoader::ClassLoader(const char *fileName) {
    this->fileName = (const char *)MjvmHeap::malloc(strlen(fileName) + 1);
    strcpy((char *)this->fileName, fileName);
    ClassFile file(fileName);
    load(file);
    file.close();
}

void ClassLoader::load(ClassFile &file) {
    magic = file.readUInt32();
    minorVersion = file.readUInt16();
    majorVersion = file.readUInt16();
    poolCount = file.readUInt16() - 1;
    poolTable = (ConstPool *)MjvmHeap::malloc(poolCount * sizeof(ConstPool));
    for(uint32_t i = 0; i < poolCount; i++) {
        *(ConstPoolTag *)&poolTable[i].tag = (ConstPoolTag)file.readUInt8();
        switch(poolTable[i].tag) {
            case CONST_UTF8: {
                uint16_t length = file.readUInt16();
                *(uint32_t *)&poolTable[i].value = (uint32_t)MjvmHeap::malloc(sizeof(ConstUtf8) + length);
                new ((ConstUtf8 *)poolTable[i].value)ConstUtf8(length);
                file.read((char *)((ConstUtf8 *)poolTable[i].value)->text, length);
                break;
            }
            case CONST_INTEGER:
            case CONST_FLOAT:
                *(uint32_t *)&poolTable[i].value = file.readUInt32();
                break;
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                ((uint16_t *)&poolTable[i].value)[0] = file.readUInt16();
                ((uint16_t *)&poolTable[i].value)[1] = file.readUInt16();
                break;
            case CONST_LONG:
            case CONST_DOUBLE:
                *(uint32_t *)&poolTable[i].value = (uint32_t)MjvmHeap::malloc(sizeof(long));
                *(uint64_t *)poolTable[i].value = file.readUInt64();
                i++;
                break;
            case CONST_CLASS:
            case CONST_STRING:
            case CONST_METHOD_TYPE:
                *(uint32_t *)&poolTable[i].value = file.readUInt16();
                break;
            case CONST_METHOD_HANDLE:
                *(uint8_t *)&poolTable[i].tag |= 0x80;
                ((uint8_t *)poolTable[i].value)[0] = file.readUInt8();
                ((uint16_t *)poolTable[i].value)[1] = file.readUInt16();
                break;
            default:
                throw "uknow pool type";
        }
    }
    accessFlags = file.readUInt16();
    thisClass = file.readUInt16();
    superClass = file.readUInt16();
    interfacesCount = file.readUInt16();
    if(interfacesCount) {
        interfaces = (uint16_t *)MjvmHeap::malloc(interfacesCount * sizeof(uint16_t));
        file.read(interfaces, interfacesCount * sizeof(uint16_t));
    }
    fieldsCount = file.readUInt16();
    if(fieldsCount) {
        fields = (FieldInfo *)MjvmHeap::malloc(fieldsCount * sizeof(FieldInfo));
        for(uint16_t i = 0; i < fieldsCount; i++) {
            FieldAccessFlag flag = (FieldAccessFlag)file.readUInt16();
            uint16_t fieldsNameIndex = file.readUInt16();
            uint16_t fieldsDescriptorIndex = file.readUInt16();
            uint16_t fieldsAttributesCount = file.readUInt16();
            new (&fields[i])FieldInfo(*this, flag, getConstUtf8(fieldsNameIndex), getConstUtf8(fieldsDescriptorIndex));
            AttributeInfo **fieldAttributes = (AttributeInfo **)MjvmHeap::malloc(fieldsAttributesCount * sizeof(AttributeInfo *));
            fields[i].setAttributes(fieldAttributes, fieldsAttributesCount);
            for(uint16_t attrIdx = 0; attrIdx < fieldsAttributesCount; attrIdx++)
                fieldAttributes[attrIdx] = &readAttribute(file);
        }
    }
    methodsCount = file.readUInt16();
    if(methodsCount) {
        methods = (MethodInfo *)MjvmHeap::malloc(methodsCount * sizeof(MethodInfo));
        for(uint16_t i = 0; i < methodsCount; i++) {
            MethodAccessFlag flag = (MethodAccessFlag)file.readUInt16();
            uint16_t methodNameIndex = file.readUInt16();
            uint16_t methodDescriptorIndex = file.readUInt16();
            uint16_t methodAttributesCount = file.readUInt16();
            new (&methods[i])MethodInfo(*this, flag, getConstUtf8(methodNameIndex), getConstUtf8(methodDescriptorIndex));
            AttributeInfo **methodAttributes = (AttributeInfo **)MjvmHeap::malloc(methodAttributesCount * sizeof(AttributeInfo *));
            methods[i].setAttributes(methodAttributes, methodAttributesCount);
            for(uint16_t attrIdx = 0; attrIdx < methodAttributesCount; attrIdx++)
                methodAttributes[attrIdx] = &readAttribute(file);
        }
    }
    attributesCount = file.readUInt16();
    if(attributesCount) {
        attributes = (AttributeInfo **)MjvmHeap::malloc(attributesCount * sizeof(AttributeInfo *));
        for(uint16_t attrIdx = 0; attrIdx < attributesCount; attrIdx++)
            attributes[attrIdx] = &readAttribute(file);
    }
}

AttributeInfo &ClassLoader::readAttribute(ClassFile &file) {
    uint16_t nameIndex = file.readUInt16();
    uint32_t length = file.readUInt32();
    AttributeType type = AttributeInfo::parseAttributeType(getConstUtf8(nameIndex));
    switch(type) {
        case ATTRIBUTE_CODE:
            return readAttributeCode(file);
        case ATTRIBUTE_LINE_NUMBER_TABLE:
            return readAttributeLineNumberTable(file);
        case ATTRIBUTE_LOCAL_VARIABLE_TABLE:
            return readAttributeLocalVariableTable(file);
        default:
            break;
    }
    AttributeRaw *attribute = (AttributeRaw *)MjvmHeap::malloc(sizeof(AttributeRaw) + length);
    new (attribute)AttributeRaw(type, length);
    file.read((uint8_t *)attribute->raw, length);
    return *attribute;
}

AttributeInfo &ClassLoader::readAttributeCode(ClassFile &file) {
    AttributeCode *attribute = (AttributeCode *)MjvmHeap::malloc(sizeof(AttributeCode));
    uint16_t maxStack = file.readUInt16();
    uint16_t maxLocals = file.readUInt16();
    uint32_t codeLength = file.readUInt32();
    uint8_t *code = (uint8_t *)MjvmHeap::malloc(codeLength);
    new (attribute)AttributeCode(maxStack, maxLocals);
    file.read(code, codeLength);
    attribute->setCode(code, codeLength);
    uint16_t exceptionTableLength = file.readUInt16();
    if(exceptionTableLength) {
        ExceptionTable *exceptionTable = (ExceptionTable *)MjvmHeap::malloc(exceptionTableLength * sizeof(ExceptionTable));
        attribute->setExceptionTable(exceptionTable, exceptionTableLength);
        for(uint16_t i = 0; i < exceptionTableLength; i++) {
            uint16_t startPc = file.readUInt16();
            uint16_t endPc = file.readUInt16();
            uint16_t handlerPc = file.readUInt16();
            uint16_t catchType = file.readUInt16();
            new (&attribute[i])ExceptionTable(startPc, endPc, handlerPc, getConstUtf8(catchType));
        }
    }
    uint16_t attrbutesCount = file.readUInt16();
    if(attrbutesCount) {
        AttributeInfo **codeAttributes = (AttributeInfo **)MjvmHeap::malloc(attrbutesCount * sizeof(AttributeInfo *));
        attribute->setAttributes(codeAttributes, attrbutesCount);
        for(uint16_t i = 0; i < attrbutesCount; i++)
            codeAttributes[i] = &readAttribute(file);
    }
    return *attribute;
}

AttributeInfo &ClassLoader::readAttributeLineNumberTable(ClassFile &file) {
    uint16_t lineNumberTableLength = file.readUInt16();
    AttributeLineNumberTable *attribute = (AttributeLineNumberTable *)MjvmHeap::malloc(sizeof(AttributeLineNumberTable) + lineNumberTableLength * sizeof(LineNumber));
    new (attribute)AttributeLineNumberTable(lineNumberTableLength);
    for(uint16_t i = 0; i < lineNumberTableLength; i++) {
        uint16_t startPc = file.readUInt16();
        uint16_t lineNumber = file.readUInt16();
        new ((LineNumber *)&attribute->getLineNumber(i))LineNumber(startPc, lineNumber);
    }
    return *attribute;
}

AttributeInfo &ClassLoader::readAttributeLocalVariableTable(ClassFile &file) {
    uint16_t localVariableTableLength = file.readUInt16();
    AttributeLocalVariableTable *attribute = (AttributeLocalVariableTable *)MjvmHeap::malloc(sizeof(AttributeLocalVariableTable) + localVariableTableLength * sizeof(LocalVariable));
    new (attribute)AttributeLocalVariableTable(localVariableTableLength);
    for(uint16_t i = 0; i < localVariableTableLength; i++) {
        uint16_t startPc = file.readUInt16();
        uint16_t length = file.readUInt16();
        uint16_t nameIndex = file.readUInt16();
        uint16_t descriptorIndex = file.readUInt16();
        uint16_t index = file.readUInt16();
        new ((LocalVariable *)&attribute->getLocalVariable(i))LocalVariable(startPc, length, getConstUtf8(nameIndex), getConstUtf8(descriptorIndex), index);
    }
    return *attribute;
}

const uint32_t ClassLoader::getMagic(void) const {
    return magic;
}

const uint16_t ClassLoader::getMinorVersion(void) const {
    return minorVersion;
}

const uint16_t ClassLoader::getMajorversion(void) const {
    return majorVersion;
}

const ConstPool &ClassLoader::getConstPool(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount)
        return poolTable[poolIndex];
    throw "index for const pool is invalid";
}

const int32_t ClassLoader::getConstInteger(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_INTEGER)
        return (int32_t)poolTable[poolIndex].value;
    throw "index for const integer is invalid";
}

const int32_t ClassLoader::getConstInteger(const ConstPool &constPool) const {
    if(constPool.tag == CONST_INTEGER)
        return (int32_t)constPool.value;
    throw "const pool tag is not integer tag";
}

const float ClassLoader::getConstFloat(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_FLOAT)
        return *(float *)&poolTable[poolIndex].value;
    throw "index for const float is invalid";
}

const float ClassLoader::getConstFloat(const ConstPool &constPool) const {
    if(constPool.tag == CONST_FLOAT)
        return *(float *)&constPool.value;
    throw "const pool tag is not float tag";
}

const int64_t ClassLoader::getConstLong(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_LONG)
        return *(int64_t *)poolTable[poolIndex].value;
    throw "index for const long is invalid";
}

const int64_t ClassLoader::getConstLong(const ConstPool &constPool) const {
    if(constPool.tag == CONST_LONG)
        return *(int64_t *)constPool.value;
    throw "const pool tag is not long tag";
}

const double ClassLoader::getConstDouble(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_DOUBLE)
        return *(double *)poolTable[poolIndex].value;
    throw "index for const double is invalid";
}

const double ClassLoader::getConstDouble(const ConstPool &constPool) const {
    if(constPool.tag == CONST_DOUBLE)
        return *(double *)constPool.value;
    throw "const pool tag is not double tag";
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

const ConstUtf8 &ClassLoader::getConstString(uint16_t poolIndex) const {
    poolIndex--;
    if(poolIndex < poolCount && poolTable[poolIndex].tag == CONST_STRING)
        return getConstUtf8(poolTable[poolIndex].value);
    throw "index for const string is invalid";
}

const ConstUtf8 &ClassLoader::getConstString(const ConstPool &constPool) const {
    if(constPool.tag == CONST_STRING)
        return getConstUtf8(constPool.value);
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
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)MjvmHeap::malloc(sizeof(ConstNameAndType));
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
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)MjvmHeap::malloc(sizeof(ConstField));
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
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)MjvmHeap::malloc(sizeof(ConstMethod));
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
            *(uint32_t *)&poolTable[poolIndex].value = (uint32_t)MjvmHeap::malloc(sizeof(ConstInterfaceMethod));
            new ((ConstInterfaceMethod *)poolTable[poolIndex].value)ConstInterfaceMethod(getConstClass(classNameIndex), getConstNameAndType(nameAndTypeIndex));
        }
        return *(ConstInterfaceMethod *)poolTable[poolIndex].value;
    }
    throw "index for const interface method is invalid";
}

const ConstInterfaceMethod &ClassLoader::getConstInterfaceMethod(const ConstPool &constPool) const {
    return getConstInterfaceMethod((uint16_t)(&constPool - poolTable) + 1);
}

const ClassAccessFlag ClassLoader::getAccessFlag(void) const {
    return (ClassAccessFlag)accessFlags;
}

const ConstUtf8 &ClassLoader::getThisClass(void) const {
    return getConstClass(thisClass);
}

const ConstUtf8 &ClassLoader::getSupperClass(void) const {
    if(superClass == 0)
        return *(ConstUtf8 *)0;
    return getConstClass(superClass);
}

const ConstUtf8 &ClassLoader::getInterface(uint8_t interfaceIndex) const {
    if(interfaceIndex < interfacesCount)
        return getConstClass(interfaces[interfaceIndex]);
    throw "index for const interface is invalid";
}

const FieldInfo &ClassLoader::getFieldInfo(uint8_t fieldIndex) const {
    if(fieldIndex < fieldsCount)
        return fields[fieldIndex];
    throw "index for field info is invalid";
}

const FieldInfo &ClassLoader::getFieldInfo(const ConstNameAndType &field) const {
    for(uint16_t i = 0; i < fieldsCount; i++) {
        if(&field.name == &fields[i].name && &field.descriptor == &fields[i].descriptor)
            return fields[i];
        else if(field.name.length == fields[i].name.length && field.descriptor.length == fields[i].descriptor.length) {
            if(
                strncmp(field.name.getText(), fields[i].name.getText(), field.name.length) == 0 &&
                strncmp(field.descriptor.getText(), fields[i].descriptor.getText(), field.descriptor.length) == 0
            )
                return fields[i];
        }
    }
    throw "can't find the field";
}

const MethodInfo &ClassLoader::getMethodInfo(uint8_t methodIndex) const {
    if(methodIndex < methodsCount)
        return methods[methodIndex];
    throw "index for method info is invalid";
}

const MethodInfo &ClassLoader::getMethodInfo(const ConstNameAndType &method) const {
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(&method.name == &methods[i].name && &method.descriptor == &methods[i].descriptor)
            return methods[i];
        else if(method.name.length == methods[i].name.length && method.descriptor.length == methods[i].descriptor.length) {
            if(
                strncmp(method.name.text, methods[i].name.text, method.name.length) == 0 &&
                strncmp(method.descriptor.text, methods[i].descriptor.text, method.descriptor.length) == 0
            )
                return methods[i];
        }
    }
    throw "can't find the method";
}

const MethodInfo &ClassLoader::getMainMethodInfo(void) const {
    for(uint16_t i = 0; i < methodsCount; i++) {
        if(methods[i].name.length == (sizeof("main") - 1) && methods[i].descriptor.length == (sizeof("([Ljava/lang/String;)V") - 1)) {
            if(
                strncmp(methods[i].name.text, "main", sizeof("main") - 1) == 0 &&
                strncmp(methods[i].descriptor.text, "([Ljava/lang/String;)V", sizeof("([Ljava/lang/String;)V") - 1) == 0
            ) {
                if((methods[i].accessFlag & (METHOD_PUBLIC | METHOD_STATIC)) == (METHOD_PUBLIC | METHOD_STATIC))
                    return methods[i];
                throw "main must be static public method";
            }
        }
    }
    throw "can't find the method";
}

ClassLoader::~ClassLoader(void) {
    MjvmHeap::free((void *)fileName);
    for(uint32_t i = 0; i < poolCount; i++) {
        switch (poolTable[i].tag) {
            case CONST_UTF8:
            case CONST_FIELD:
            case CONST_METHOD:
            case CONST_INTERFACE_METHOD:
            case CONST_NAME_AND_TYPE:
            case CONST_INVOKE_DYNAMIC:
                MjvmHeap::free((void *)poolTable[i].value);
                break;
            case CONST_LONG:
            case CONST_DOUBLE:
                MjvmHeap::free((void *)poolTable[i].value);
                i++;
                break;
            case CONST_METHOD_HANDLE:
                MjvmHeap::free((void *)poolTable[i].value);
                break;
            default:
                break;
        }
    }
    MjvmHeap::free(poolTable);
    if(interfacesCount)
        MjvmHeap::free(interfaces);
    if(fieldsCount) {
        for(uint32_t i = 0; i < fieldsCount; i++)
            fields[i].~FieldInfo();
        MjvmHeap::free(fields);
    }
    if(methodsCount) {
        for(uint32_t i = 0; i < methodsCount; i++)
            methods[i].~MethodInfo();
        MjvmHeap::free(methods);
    }
    if(attributesCount) {
        for(uint32_t i = 0; i < attributesCount; i++) {
            attributes[i]->~AttributeInfo();
            MjvmHeap::free(attributes[i]);
        }
        MjvmHeap::free(attributes);
    }
}
