
#include <iostream>
#include <string.h>
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

    uint16_t field32Index = fields32Count;
    uint16_t field64Index = fields64Count;
    uint16_t fieldObjIndex = fieldsObjCount;

    loader = &classLoader;
    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(int16_t index = fieldsCount - 1; index >= 0; index--) {
            const FlintFieldInfo &fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo.accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo.descriptor.text[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fieldsData64[--field64Index])FlintFieldData64(fieldInfo);
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        new (&fieldsObject[--fieldObjIndex])FlintFieldObject(fieldInfo);
                        break;
                    default:
                        new (&fieldsData32[--field32Index])FlintFieldData32(fieldInfo);
                        break;
                }
            }
        }
        FlintConstUtf8 *superClass = &loader->getSuperClass();
        loader = superClass ? &flint.load(*superClass) : 0;
    }
}

FlintFieldData32 &FlintFieldsData::getFieldData32(const char *fieldName, uint32_t *index) const {
    if(index && *index)
        return fieldsData32[*index & 0x7FFFFFFF];
    if(fields32Count) {
        uint16_t length = strlen(fieldName);
        uint32_t hash = Flint_CalcHash(fieldName, length, false);

        for(uint16_t i = 0; i < fields32Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(CONST_UTF8_HASH(fieldInfo.name) == hash) {
                if(strncmp(fieldInfo.name.text, fieldName, length) == 0) {
                    if(index)
                        *index = i | 0x80000000;
                    return fieldsData32[i];
                }
            }
        }
    }
    return *(FlintFieldData32 *)0;
}

FlintFieldData32 &FlintFieldsData::getFieldData32(const FlintConstUtf8 &fieldName, uint32_t *index) const {
    if(index && *index)
        return fieldsData32[*index & 0x7FFFFFFF];
    for(uint16_t i = 0; i < fields32Count; i++) {
        const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
        if(fieldInfo.name == fieldName) {
            if(index)
                *index = i | 0x80000000;
            return fieldsData32[i];
        }
    }
    return *(FlintFieldData32 *)0;
}

FlintFieldData32 &FlintFieldsData::getFieldData32(FlintConstField &constField) const {
    if(constField.fieldIndex == 0 && fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.name == constField.nameAndType.name && fieldInfo.descriptor == constField.nameAndType.descriptor)
                constField.fieldIndex = i | 0x80000000;
        }
        if(constField.fieldIndex == 0)
            return *(FlintFieldData32 *)0;
    }
    return fieldsData32[constField.fieldIndex & 0x7FFFFFFF];
}

FlintFieldData32 &FlintFieldsData::getFieldData32ByIndex(int32_t index) const {
    return fieldsData32[index];
}

FlintFieldData64 &FlintFieldsData::getFieldData64(const char *fieldName, uint32_t *index) const {
    if(index && *index)
        return fieldsData64[*index & 0x7FFFFFFF];
    if(fields64Count) {
        uint16_t length = strlen(fieldName);
        uint32_t hash = Flint_CalcHash(fieldName, length, false);

        for(uint16_t i = 0; i < fields64Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(CONST_UTF8_HASH(fieldInfo.name) == hash) {
                if(strncmp(fieldInfo.name.text, fieldName, length) == 0) {
                    if(index)
                        *index = i | 0x80000000;
                    return fieldsData64[i];
                }
            }
        }
    }
    return *(FlintFieldData64 *)0;
}

FlintFieldData64 &FlintFieldsData::getFieldData64(const FlintConstUtf8 &fieldName, uint32_t *index) const {
    if(index && *index)
        return fieldsData64[*index & 0x7FFFFFFF];
    for(uint16_t i = 0; i < fields64Count; i++) {
        const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
        if(fieldInfo.name == fieldName) {
            if(index)
                *index = i | 0x80000000;
            return fieldsData64[i];
        }
    }
    return *(FlintFieldData64 *)0;
}

FlintFieldData64 &FlintFieldsData::getFieldData64(FlintConstField &constField) const {
    if(constField.fieldIndex == 0 && fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.name == constField.nameAndType.name && fieldInfo.descriptor == constField.nameAndType.descriptor)
                constField.fieldIndex = i | 0x80000000;
        }
        if(constField.fieldIndex == 0)
            return *(FlintFieldData64 *)0;
    }
    return fieldsData64[constField.fieldIndex & 0x7FFFFFFF];
}

FlintFieldData64 &FlintFieldsData::getFieldData64ByIndex(int32_t index) const {
    return fieldsData64[index];
}

FlintFieldObject &FlintFieldsData::getFieldObject(const char *fieldName, uint32_t *index) const {
    if(index && *index)
        return fieldsObject[*index & 0x7FFFFFFF];
    if(fieldsObjCount) {
        uint16_t length = strlen(fieldName);
        uint32_t hash = Flint_CalcHash(fieldName, length, false);

        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(CONST_UTF8_HASH(fieldInfo.name) == hash) {
                if(strncmp(fieldInfo.name.text, fieldName, length) == 0) {
                    if(index)
                        *index = i | 0x80000000;
                    return fieldsObject[i];
                }
            }
        }
    }
    return *(FlintFieldObject *)0;
}

FlintFieldObject &FlintFieldsData::getFieldObject(const FlintConstUtf8 &fieldName, uint32_t *index) const {
    if(index && *index)
        return fieldsObject[*index & 0x7FFFFFFF];
    for(uint16_t i = 0; i < fieldsObjCount; i++) {
        const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
        if(fieldInfo.name == fieldName) {
            if(index)
                *index = i | 0x80000000;
            return fieldsObject[i];
        }
    }
    return *(FlintFieldObject *)0;
}

FlintFieldObject &FlintFieldsData::getFieldObject(FlintConstField &constField) const {
    if(constField.fieldIndex == 0 && fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.name == constField.nameAndType.name && fieldInfo.descriptor == constField.nameAndType.descriptor)
                constField.fieldIndex = i | 0x80000000;
        }
        if(constField.fieldIndex == 0)
            return *(FlintFieldObject *)0;
    }
    return fieldsObject[constField.fieldIndex & 0x7FFFFFFF];
}

FlintFieldObject &FlintFieldsData::getFieldObjectByIndex(int32_t index) const {
    return fieldsObject[index];
}

FlintFieldsData::~FlintFieldsData(void) {
    if(fieldsData32)
        Flint::free(fieldsData32);
    if(fieldsData64)
        Flint::free(fieldsData64);
    if(fieldsObject)
        Flint::free(fieldsObject);
}
