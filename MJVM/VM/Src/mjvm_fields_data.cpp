
#include <iostream>
#include "mjvm.h"
#include "mjvm_fields_data.h"
#include "mjvm_execution.h"

MjvmFieldData32::MjvmFieldData32(const MjvmFieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

MjvmFieldData64::MjvmFieldData64(const MjvmFieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

MjvmFieldObject::MjvmFieldObject(const MjvmFieldInfo &fieldInfo) : fieldInfo(fieldInfo), object(0) {

}

MjvmFieldsData::MjvmFieldsData(MjvmExecution &execution, const MjvmClassLoader &classLoader, bool isStatic) :
fields32Count(0), fields64Count(0), fieldsObjCount(0) {
    if(isStatic)
        loadStatic(classLoader);
    else
        loadNonStatic(execution, classLoader);
}

void MjvmFieldsData::loadStatic(const MjvmClassLoader &classLoader) {
    uint16_t fieldsCount = classLoader.getFieldsCount();
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        const MjvmFieldInfo &fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo.accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo.descriptor.text[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    (*(uint32_t *)&fields64Count)++;
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    (*(uint32_t *)&fieldsObjCount)++;
                    break;
                default:
                    (*(uint32_t *)&fields32Count)++;
                    break;
            }
        }
    }

    fieldsData32 = (fields32Count) ? (MjvmFieldData32 *)Mjvm::malloc(fields32Count * sizeof(MjvmFieldData32)) : 0;
    fieldsData64 = (fields64Count) ? (MjvmFieldData64 *)Mjvm::malloc(fields64Count * sizeof(MjvmFieldData64)) : 0;
    fieldsObject = (fieldsObjCount) ? (MjvmFieldObject *)Mjvm::malloc(fieldsObjCount * sizeof(MjvmFieldObject)) : 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        const MjvmFieldInfo &fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo.accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo.descriptor.text[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    new (&fieldsData64[field64Index++])MjvmFieldData64(fieldInfo);
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    new (&fieldsObject[fieldObjIndex++])MjvmFieldObject(fieldInfo);
                    break;
                default:
                    new (&fieldsData32[field32Index++])MjvmFieldData32(fieldInfo);
                    break;
            }
        }
    }
}

void MjvmFieldsData::loadNonStatic(MjvmExecution &execution, const MjvmClassLoader &classLoader) {
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    const MjvmClassLoader *loader = &classLoader;

    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            const MjvmFieldInfo &fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo.accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo.descriptor.text[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        (*(uint32_t *)&fields64Count)++;
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        (*(uint32_t *)&fieldsObjCount)++;
                        break;
                    default:
                        (*(uint32_t *)&fields32Count)++;
                        break;
                }
            }
        }
        MjvmConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &execution.load(*superClass) : 0;
    }

    fieldsData32 = (fields32Count) ? (MjvmFieldData32 *)Mjvm::malloc(fields32Count * sizeof(MjvmFieldData32)) : 0;
    fieldsData64 = (fields64Count) ? (MjvmFieldData64 *)Mjvm::malloc(fields64Count * sizeof(MjvmFieldData64)) : 0;
    fieldsObject = (fieldsObjCount) ? (MjvmFieldObject *)Mjvm::malloc(fieldsObjCount * sizeof(MjvmFieldObject)) : 0;

    loader = &classLoader;
    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            const MjvmFieldInfo &fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo.accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo.descriptor.text[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fieldsData64[field64Index++])MjvmFieldData64(fieldInfo);
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        new (&fieldsObject[fieldObjIndex++])MjvmFieldObject(fieldInfo);
                        break;
                    default:
                        new (&fieldsData32[field32Index++])MjvmFieldData32(fieldInfo);
                        break;
                }
            }
        }
        MjvmConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &execution.load(*superClass) : 0;
    }
}

MjvmFieldData32 &MjvmFieldsData::getFieldData32(const MjvmConstUtf8 &fieldName) const {
    if(fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const MjvmFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.name == fieldName)
                return fieldsData32[i];
        }
    }
    return *(MjvmFieldData32 *)0;
}

MjvmFieldData32 &MjvmFieldsData::getFieldData32(const MjvmConstNameAndType &fieldNameAndType) const {
    if(fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const MjvmFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.name == fieldNameAndType.name && fieldInfo.descriptor == fieldNameAndType.descriptor)
                return fieldsData32[i];
        }
    }
    return *(MjvmFieldData32 *)0;
}

MjvmFieldData64 &MjvmFieldsData::getFieldData64(const MjvmConstUtf8 &fieldName) const {
    if(fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const MjvmFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.name == fieldName)
                return fieldsData64[i];
        }
    }
    return *(MjvmFieldData64 *)0;
}

MjvmFieldData64 &MjvmFieldsData::getFieldData64(const MjvmConstNameAndType &fieldNameAndType) const {
    if(fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const MjvmFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.name == fieldNameAndType.name && fieldInfo.descriptor == fieldNameAndType.descriptor)
                return fieldsData64[i];
        }
    }
    return *(MjvmFieldData64 *)0;
}

MjvmFieldObject &MjvmFieldsData::getFieldObject(const MjvmConstUtf8 &fieldName) const {
    if(fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const MjvmFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.name == fieldName)
                return fieldsObject[i];
        }
    }
    return *(MjvmFieldObject *)0;
}

MjvmFieldObject &MjvmFieldsData::getFieldObject(const MjvmConstNameAndType &fieldNameAndType) const {
    if(fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const MjvmFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.name == fieldNameAndType.name && fieldInfo.descriptor == fieldNameAndType.descriptor)
                return fieldsObject[i];
        }
    }
    return *(MjvmFieldObject *)0;
}

MjvmFieldsData::~MjvmFieldsData(void) {
    if(fieldsData32)
        Mjvm::free(fieldsData32);
    if(fieldsData64)
        Mjvm::free(fieldsData64);
    if(fieldsObject)
        Mjvm::free(fieldsObject);
}

ClassData::~ClassData() {
    if(staticFiledsData) {
        staticFiledsData->~MjvmFieldsData();
        Mjvm::free(staticFiledsData);
    }
}

ClassData::ClassData(const char *fileName) : MjvmClassLoader(fileName) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFiledsData = 0;
    next = 0;
}

ClassData::ClassData(const char *fileName, uint16_t length) : MjvmClassLoader(fileName, length) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFiledsData = 0;
    next = 0;
}

ClassData::ClassData(const MjvmConstUtf8 &fileName) : MjvmClassLoader(fileName) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFiledsData = 0;
    next = 0;
}
