
#include <iostream>
#include <string.h>
#include "flint.h"
#include "flint_fields_data.h"

FlintFieldData32::FlintFieldData32(const FlintFieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FlintFieldData64::FlintFieldData64(const FlintFieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FlintFieldObject::FlintFieldObject(const FlintFieldInfo &fieldInfo) : fieldInfo(fieldInfo), object(NULL_PTR) {

}

FlintFieldsData::FlintFieldsData(void) : fields32Count(0), fields64Count(0), fieldsObjCount(0) {

}

FlintResult<FlintClassLoader> FlintFieldsData::loadStatic(FlintClassLoader &classLoader) {
    uint16_t fieldsCount = classLoader.getFieldsCount();
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        FlintFieldInfo *fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo->accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo->getDescriptor().text[0]) {
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

    if(fields32Count) {
        fieldsData32 = (FlintFieldData32 *)Flint::malloc(fields32Count * sizeof(FlintFieldData32));
        if(fieldsData32 == NULL_PTR)
            return FlintResult<FlintClassLoader>(ERR_OUT_OF_MEMORY, classLoader.thisClass->text, classLoader.thisClass->length);
    }
    else
        fieldsData32 = NULL_PTR;

    if(fields64Count) {
        fieldsData64 = (FlintFieldData64 *)Flint::malloc(fields64Count * sizeof(FlintFieldData64));
        if(fieldsData64 == NULL_PTR)
            return FlintResult<FlintClassLoader>(ERR_OUT_OF_MEMORY, classLoader.thisClass->text, classLoader.thisClass->length);
    }
    else
        fieldsData64 = NULL_PTR;

    if(fieldsObjCount) {
        fieldsObject = (FlintFieldObject *)Flint::malloc(fieldsObjCount * sizeof(FlintFieldObject));
        if(fieldsObject == NULL_PTR)
            return FlintResult<FlintClassLoader>(ERR_OUT_OF_MEMORY, classLoader.thisClass->text, classLoader.thisClass->length);
    }
    else
        fieldsObject = NULL_PTR;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        FlintFieldInfo *fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo->accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo->getDescriptor().text[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    new (&fieldsData64[field64Index++])FlintFieldData64(*fieldInfo);
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    new (&fieldsObject[fieldObjIndex++])FlintFieldObject(*fieldInfo);
                    break;
                default:
                    new (&fieldsData32[field32Index++])FlintFieldData32(*fieldInfo);
                    break;
            }
        }
    }

    return &classLoader;
}

FlintResult<FlintClassLoader> FlintFieldsData::loadNonStatic(Flint &flint, FlintClassLoader &classLoader) {
    FlintClassLoader *loader = &classLoader;

    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            FlintFieldInfo *fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo->accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo->getDescriptor().text[0]) {
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
        FlintConstUtf8 *superClass = loader->superClass;
        if(!superClass)
            break;
        auto tmp = flint.load(*superClass);
        if(tmp.err != ERR_OK)
            return tmp;
        loader = tmp.value;
    }

    if(fields32Count) {
        fieldsData32 = (FlintFieldData32 *)Flint::malloc(fields32Count * sizeof(FlintFieldData32));
        if(fieldsData32 == NULL_PTR)
            return FlintResult<FlintClassLoader>(ERR_OUT_OF_MEMORY, classLoader.thisClass->text, classLoader.thisClass->length);
    }
    else
        fieldsData32 = NULL_PTR;

    if(fields64Count) {
        fieldsData64 = (FlintFieldData64 *)Flint::malloc(fields64Count * sizeof(FlintFieldData64));
        if(fieldsData64 == NULL_PTR)
            return FlintResult<FlintClassLoader>(ERR_OUT_OF_MEMORY, classLoader.thisClass->text, classLoader.thisClass->length);
    }
    else
        fieldsData64 = NULL_PTR;

    if(fieldsObjCount) {
        fieldsObject = (FlintFieldObject *)Flint::malloc(fieldsObjCount * sizeof(FlintFieldObject));
        if(fieldsObject == NULL_PTR)
            return FlintResult<FlintClassLoader>(ERR_OUT_OF_MEMORY, classLoader.thisClass->text, classLoader.thisClass->length);
    }
    else
        fieldsObject = NULL_PTR;

    uint16_t field32Index = fields32Count;
    uint16_t field64Index = fields64Count;
    uint16_t fieldObjIndex = fieldsObjCount;

    loader = &classLoader;
    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(int16_t index = fieldsCount - 1; index >= 0; index--) {
            FlintFieldInfo *fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo->accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo->getDescriptor().text[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fieldsData64[--field64Index])FlintFieldData64(*fieldInfo);
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        new (&fieldsObject[--fieldObjIndex])FlintFieldObject(*fieldInfo);
                        break;
                    default:
                        new (&fieldsData32[--field32Index])FlintFieldData32(*fieldInfo);
                        break;
                }
            }
        }
        FlintConstUtf8 *superClass = loader->superClass;
        if(!superClass)
            break;
        auto tmp = flint.load(*superClass);
        if(tmp.err != ERR_OK)
            return tmp;
        loader = tmp.value;
    }

    return &classLoader;
}

FlintFieldData32 *FlintFieldsData::getFieldData32(const char *fieldName, uint32_t *index) const {
    if(index && *index)
        return &fieldsData32[*index & 0x7FFFFFFF];
    if(fields32Count) {
        uint16_t length = strlen(fieldName);
        uint32_t hash = Flint_CalcHash(fieldName, length, false);

        for(uint16_t i = 0; i < fields32Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(CONST_UTF8_HASH(fieldInfo.getName()) == hash) {
                if(strncmp(fieldInfo.getName().text, fieldName, length) == 0) {
                    if(index)
                        *index = i | 0x80000000;
                    return &fieldsData32[i];
                }
            }
        }
    }
    return NULL_PTR;
}

FlintFieldData32 *FlintFieldsData::getFieldData32(FlintConstUtf8 &fieldName, uint32_t *index) const {
    if(index && *index)
        return &fieldsData32[*index & 0x7FFFFFFF];
    for(uint16_t i = 0; i < fields32Count; i++) {
        const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
        if(fieldInfo.getName() == fieldName) {
            if(index)
                *index = i | 0x80000000;
            return &fieldsData32[i];
        }
    }
    return NULL_PTR;
}

FlintFieldData32 *FlintFieldsData::getFieldData32(FlintConstField &constField) const {
    if(constField.fieldIndex == 0 && fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.getName() == constField.nameAndType.name && fieldInfo.getDescriptor() == constField.nameAndType.descriptor)
                constField.fieldIndex = i | 0x80000000;
        }
        if(constField.fieldIndex == 0)
            return NULL_PTR;
    }
    return &fieldsData32[constField.fieldIndex & 0x7FFFFFFF];
}

FlintFieldData32 *FlintFieldsData::getFieldData32ByIndex(int32_t index) const {
    return &fieldsData32[index];
}

FlintFieldData64 *FlintFieldsData::getFieldData64(const char *fieldName, uint32_t *index) const {
    if(index && *index)
        return &fieldsData64[*index & 0x7FFFFFFF];
    if(fields64Count) {
        uint16_t length = strlen(fieldName);
        uint32_t hash = Flint_CalcHash(fieldName, length, false);

        for(uint16_t i = 0; i < fields64Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(CONST_UTF8_HASH(fieldInfo.getName()) == hash) {
                if(strncmp(fieldInfo.getName().text, fieldName, length) == 0) {
                    if(index)
                        *index = i | 0x80000000;
                    return &fieldsData64[i];
                }
            }
        }
    }
    return NULL_PTR;
}

FlintFieldData64 *FlintFieldsData::getFieldData64(FlintConstUtf8 &fieldName, uint32_t *index) const {
    if(index && *index)
        return &fieldsData64[*index & 0x7FFFFFFF];
    for(uint16_t i = 0; i < fields64Count; i++) {
        const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
        if(fieldInfo.getName() == fieldName) {
            if(index)
                *index = i | 0x80000000;
            return &fieldsData64[i];
        }
    }
    return NULL_PTR;
}

FlintFieldData64 *FlintFieldsData::getFieldData64(FlintConstField &constField) const {
    if(constField.fieldIndex == 0 && fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const FlintFieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.getName() == constField.nameAndType.name && fieldInfo.getDescriptor() == constField.nameAndType.descriptor)
                constField.fieldIndex = i | 0x80000000;
        }
        if(constField.fieldIndex == 0)
            return NULL_PTR;
    }
    return &fieldsData64[constField.fieldIndex & 0x7FFFFFFF];
}

FlintFieldData64 *FlintFieldsData::getFieldData64ByIndex(int32_t index) const {
    return &fieldsData64[index];
}

FlintFieldObject *FlintFieldsData::getFieldObject(const char *fieldName, uint32_t *index) const {
    if(index && *index)
        return &fieldsObject[*index & 0x7FFFFFFF];
    if(fieldsObjCount) {
        uint16_t length = strlen(fieldName);
        uint32_t hash = Flint_CalcHash(fieldName, length, false);

        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(CONST_UTF8_HASH(fieldInfo.getName()) == hash) {
                if(strncmp(fieldInfo.getName().text, fieldName, length) == 0) {
                    if(index)
                        *index = i | 0x80000000;
                    return &fieldsObject[i];
                }
            }
        }
    }
    return NULL_PTR;
}

FlintFieldObject *FlintFieldsData::getFieldObject(FlintConstUtf8 &fieldName, uint32_t *index) const {
    if(index && *index)
        return &fieldsObject[*index & 0x7FFFFFFF];
    for(uint16_t i = 0; i < fieldsObjCount; i++) {
        const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
        if(fieldInfo.getName() == fieldName) {
            if(index)
                *index = i | 0x80000000;
            return &fieldsObject[i];
        }
    }
    return NULL_PTR;
}

FlintFieldObject *FlintFieldsData::getFieldObject(FlintConstField &constField) const {
    if(constField.fieldIndex == 0 && fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FlintFieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.getName() == constField.nameAndType.name && fieldInfo.getDescriptor() == constField.nameAndType.descriptor)
                constField.fieldIndex = i | 0x80000000;
        }
        if(constField.fieldIndex == 0)
            return NULL_PTR;
    }
    return &fieldsObject[constField.fieldIndex & 0x7FFFFFFF];
}

FlintFieldObject *FlintFieldsData::getFieldObjectByIndex(int32_t index) const {
    return &fieldsObject[index];
}

FlintFieldsData::~FlintFieldsData(void) {
    if(fieldsData32)
        Flint::free(fieldsData32);
    if(fieldsData64)
        Flint::free(fieldsData64);
    if(fieldsObject)
        Flint::free(fieldsObject);
}
