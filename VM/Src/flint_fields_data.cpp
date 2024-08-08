
#include <iostream>
#include "flint.h"
#include "flint_fields_data.h"

FlintFieldData32::FlintFieldData32(const FlintFieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FlintFieldData64::FlintFieldData64(const FlintFieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FlintFieldObject::FlintFieldObject(const FlintFieldInfo &fieldInfo) : fieldInfo(fieldInfo), object(0) {

}

FlintFieldsData::FlintFieldsData(Flint &flint, const FlintClassLoader &classLoader, bool isStatic) :
fields32Count(0), fields64Count(0), fieldsObjCount(0) {
    if(isStatic)
        loadStatic(classLoader);
    else
        loadNonStatic(flint, classLoader);
}

void FlintFieldsData::loadStatic(const FlintClassLoader &classLoader) {
    uint16_t fieldsCount = classLoader.getFieldsCount();
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        const FlintFieldInfo &fieldInfo = classLoader.getFieldInfo(index);
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

    fieldsData32 = (fields32Count) ? (FlintFieldData32 *)Flint::malloc(fields32Count * sizeof(FlintFieldData32)) : 0;
    fieldsData64 = (fields64Count) ? (FlintFieldData64 *)Flint::malloc(fields64Count * sizeof(FlintFieldData64)) : 0;
    fieldsObject = (fieldsObjCount) ? (FlintFieldObject *)Flint::malloc(fieldsObjCount * sizeof(FlintFieldObject)) : 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        const FlintFieldInfo &fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo.accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo.descriptor.text[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    new (&fieldsData64[field64Index++])FlintFieldData64(fieldInfo);
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    new (&fieldsObject[fieldObjIndex++])FlintFieldObject(fieldInfo);
                    break;
                default:
                    new (&fieldsData32[field32Index++])FlintFieldData32(fieldInfo);
                    break;
            }
        }
    }
}

void FlintFieldsData::loadNonStatic(Flint &flint, const FlintClassLoader &classLoader) {
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    const FlintClassLoader *loader = &classLoader;

    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            const FlintFieldInfo &fieldInfo = loader->getFieldInfo(index);
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
        FlintConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &flint.load(*superClass) : 0;
    }

    fieldsData32 = (fields32Count) ? (FlintFieldData32 *)Flint::malloc(fields32Count * sizeof(FlintFieldData32)) : 0;
    fieldsData64 = (fields64Count) ? (FlintFieldData64 *)Flint::malloc(fields64Count * sizeof(FlintFieldData64)) : 0;
    fieldsObject = (fieldsObjCount) ? (FlintFieldObject *)Flint::malloc(fieldsObjCount * sizeof(FlintFieldObject)) : 0;

    loader = &classLoader;
    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            const FlintFieldInfo &fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo.accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo.descriptor.text[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fieldsData64[field64Index++])FlintFieldData64(fieldInfo);
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        new (&fieldsObject[fieldObjIndex++])FlintFieldObject(fieldInfo);
                        break;
                    default:
                        new (&fieldsData32[field32Index++])FlintFieldData32(fieldInfo);
                        break;
                }
            }
        }
        FlintConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &flint.load(*superClass) : 0;
    }
}

FlintFieldData32 &FlintFieldsData::getFieldData32(const FlintConstUtf8 &fieldName) const {
    if(fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.name == fieldName)
                return fieldsData32[i];
        }
    }
    return *(FlintFieldData32 *)0;
}

FlintFieldData32 &FlintFieldsData::getFieldData32(const FlintConstNameAndType &fieldNameAndType) const {
    if(fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.name == fieldNameAndType.name && fieldInfo.descriptor == fieldNameAndType.descriptor)
                return fieldsData32[i];
        }
    }
    return *(FlintFieldData32 *)0;
}

FlintFieldData64 &FlintFieldsData::getFieldData64(const FlintConstUtf8 &fieldName) const {
    if(fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.name == fieldName)
                return fieldsData64[i];
        }
    }
    return *(FlintFieldData64 *)0;
}

FlintFieldData64 &FlintFieldsData::getFieldData64(const FlintConstNameAndType &fieldNameAndType) const {
    if(fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.name == fieldNameAndType.name && fieldInfo.descriptor == fieldNameAndType.descriptor)
                return fieldsData64[i];
        }
    }
    return *(FlintFieldData64 *)0;
}

FlintFieldObject &FlintFieldsData::getFieldObject(const FlintConstUtf8 &fieldName) const {
    if(fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.name == fieldName)
                return fieldsObject[i];
        }
    }
    return *(FlintFieldObject *)0;
}

FlintFieldObject &FlintFieldsData::getFieldObject(const FlintConstNameAndType &fieldNameAndType) const {
    if(fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.name == fieldNameAndType.name && fieldInfo.descriptor == fieldNameAndType.descriptor)
                return fieldsObject[i];
        }
    }
    return *(FlintFieldObject *)0;
}

FlintFieldsData::~FlintFieldsData(void) {
    if(fieldsData32)
        Flint::free(fieldsData32);
    if(fieldsData64)
        Flint::free(fieldsData64);
    if(fieldsObject)
        Flint::free(fieldsObject);
}

void ClassData::clearStaticFields(void) {
    if(staticFieldsData) {
        staticFieldsData->~FlintFieldsData();
        Flint::free(staticFieldsData);
        staticFieldsData = 0;
    }
}

ClassData::~ClassData() {
    clearStaticFields();
}

ClassData::ClassData( const char *fileName) : FlintClassLoader(fileName) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFieldsData = 0;
    next = 0;
}

ClassData::ClassData(const char *fileName, uint16_t length) : FlintClassLoader(fileName, length) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFieldsData = 0;
    next = 0;
}

ClassData::ClassData(const FlintConstUtf8 &fileName) : FlintClassLoader(fileName) {
    ownId = 0;
    monitorCount = 0;
    isInitializing = 0;
    staticFieldsData = 0;
    next = 0;
}
