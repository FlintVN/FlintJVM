
#include "mjvm.h"
#include "mjvm_fields_data.h"
#include "mjvm_execution.h"

FieldData8::FieldData8(const FieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FieldData16::FieldData16(const FieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FieldData32::FieldData32(const FieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FieldData64::FieldData64(const FieldInfo &fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FieldObject::FieldObject(const FieldInfo &fieldInfo) : fieldInfo(fieldInfo), object(0) {

}

FieldsData::FieldsData(Execution &execution, const ClassLoader &classLoader, bool isStatic) :
fields8Count(0), fields16Count(0), fields32Count(0), fields64Count(0), fieldsObjCount(0) {
    if(isStatic)
        loadStatic(classLoader);
    else
        loadNonStatic(execution, classLoader);
}

void FieldsData::loadStatic(const ClassLoader &classLoader) {
    uint16_t fieldsCount = classLoader.getFieldsCount();
    uint16_t field8Index = 0;
    uint16_t field16Index = 0;
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        const FieldInfo &fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo.accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            const ConstUtf8 &descriptor = fieldInfo.descriptor;
            switch(fieldInfo.descriptor.text[0]) {
                case 'Z':
                case 'B':   /* Byte */
                    (*(uint32_t *)&fields8Count)++;
                    break;
                case 'C':   /* Char */
                case 'S':   /* Short */
                    (*(uint32_t *)&fields16Count)++;
                    break;
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

    fieldsData8 = (fields8Count) ? (FieldData8 *)Mjvm::malloc(fields8Count * sizeof(FieldData8)) : 0;
    fieldsData16 = (fields16Count) ? (FieldData16 *)Mjvm::malloc(fields16Count * sizeof(FieldData16)) : 0;
    fieldsData32 = (fields32Count) ? (FieldData32 *)Mjvm::malloc(fields32Count * sizeof(FieldData32)) : 0;
    fieldsData64 = (fields64Count) ? (FieldData64 *)Mjvm::malloc(fields64Count * sizeof(FieldData64)) : 0;
    fieldsObject = (fieldsObjCount) ? (FieldObject *)Mjvm::malloc(fieldsObjCount * sizeof(FieldObject)) : 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        const FieldInfo &fieldInfo = classLoader.getFieldInfo(index);
        if((fieldInfo.accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            const ConstUtf8 &descriptor = fieldInfo.descriptor;
            switch(fieldInfo.descriptor.text[0]) {
                case 'Z':
                case 'B':   /* Byte */
                    new (&fieldsData8[field8Index++])FieldData8(fieldInfo);
                    break;
                case 'C':   /* Char */
                case 'S':   /* Short */
                    new (&fieldsData16[field16Index++])FieldData16(fieldInfo);
                    break;
                case 'J':   /* Long */
                case 'D':   /* Double */
                    new (&fieldsData64[field64Index++])FieldData64(fieldInfo);
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    new (&fieldsObject[fieldObjIndex++])FieldObject(fieldInfo);
                    break;
                default:
                    new (&fieldsData32[field32Index++])FieldData32(fieldInfo);
                    break;
            }
        }
    }
}

void FieldsData::loadNonStatic(Execution &execution, const ClassLoader &classLoader) {
    uint16_t field8Index = 0;
    uint16_t field16Index = 0;
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    const ClassLoader *loader = &classLoader;

    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            const FieldInfo &fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo.accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                const ConstUtf8 &descriptor = fieldInfo.descriptor;
                switch(fieldInfo.descriptor.text[0]) {
                    case 'Z':
                    case 'B':   /* Byte */
                        (*(uint32_t *)&fields8Count)++;
                        break;
                    case 'C':   /* Char */
                    case 'S':   /* Short */
                        (*(uint32_t *)&fields16Count)++;
                        break;
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
        const ConstUtf8 &supperClass = loader->getSupperClass();
        loader = ((int32_t)&supperClass) ? &execution.load(supperClass) : 0;
    }

    fieldsData8 = (fields8Count) ? (FieldData8 *)Mjvm::malloc(fields8Count * sizeof(FieldData8)) : 0;
    fieldsData16 = (fields16Count) ? (FieldData16 *)Mjvm::malloc(fields16Count * sizeof(FieldData16)) : 0;
    fieldsData32 = (fields32Count) ? (FieldData32 *)Mjvm::malloc(fields32Count * sizeof(FieldData32)) : 0;
    fieldsData64 = (fields64Count) ? (FieldData64 *)Mjvm::malloc(fields64Count * sizeof(FieldData64)) : 0;
    fieldsObject = (fieldsObjCount) ? (FieldObject *)Mjvm::malloc(fieldsObjCount * sizeof(FieldObject)) : 0;

    loader = &classLoader;
    while(loader) {
        uint16_t fieldsCount = loader->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            const FieldInfo &fieldInfo = loader->getFieldInfo(index);
            if((fieldInfo.accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                const ConstUtf8 &descriptor = fieldInfo.descriptor;
                switch(fieldInfo.descriptor.text[0]) {
                    case 'Z':
                    case 'B':   /* Byte */
                        new (&fieldsData8[field8Index++])FieldData8(fieldInfo);
                        break;
                    case 'C':   /* Char */
                    case 'S':   /* Short */
                        new (&fieldsData16[field16Index++])FieldData16(fieldInfo);
                        break;
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fieldsData64[field64Index++])FieldData64(fieldInfo);
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        new (&fieldsObject[fieldObjIndex++])FieldObject(fieldInfo);
                        break;
                    default:
                        new (&fieldsData32[field32Index++])FieldData32(fieldInfo);
                        break;
                }
            }
        }
        const ConstUtf8 &supperClass = loader->getSupperClass();
        loader = ((int32_t)&supperClass) ? &execution.load(supperClass) : 0;
    }
}

FieldData8 &FieldsData::getFieldData8(const ConstNameAndType &fieldName) const {
    if(fields8Count) {
        for(uint16_t i = 0; i < fields8Count; i++) {
            const FieldInfo &fieldInfo = fieldsData8[i].fieldInfo;
            if(fieldInfo.name == fieldName.name && fieldInfo.descriptor == fieldName.descriptor)
                return fieldsData8[i];
        }
    }
    return *(FieldData8 *)0;
}

FieldData16 &FieldsData::getFieldData16(const ConstNameAndType &fieldName) const {
    if(fields16Count) {
        for(uint16_t i = 0; i < fields16Count; i++) {
            const FieldInfo &fieldInfo = fieldsData16[i].fieldInfo;
            if(fieldInfo.name == fieldName.name && fieldInfo.descriptor == fieldName.descriptor)
                return fieldsData16[i];
        }
    }
    return *(FieldData16 *)0;
}

FieldData32 &FieldsData::getFieldData32(const ConstNameAndType &fieldName) const {
    if(fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            const FieldInfo &fieldInfo = fieldsData32[i].fieldInfo;
            if(fieldInfo.name == fieldName.name && fieldInfo.descriptor == fieldName.descriptor)
                return fieldsData32[i];
        }
    }
    return *(FieldData32 *)0;
}

FieldData64 &FieldsData::getFieldData64(const ConstNameAndType &fieldName) const {
    if(fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            const FieldInfo &fieldInfo = fieldsData64[i].fieldInfo;
            if(fieldInfo.name == fieldName.name && fieldInfo.descriptor == fieldName.descriptor)
                return fieldsData64[i];
        }
    }
    return *(FieldData64 *)0;
}

FieldObject &FieldsData::getFieldObject(const ConstNameAndType &fieldName) const {
    if(fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FieldInfo &fieldInfo = fieldsObject[i].fieldInfo;
            if(fieldInfo.name == fieldName.name && fieldInfo.descriptor == fieldName.descriptor)
                return fieldsObject[i];
        }
    }
    return *(FieldObject *)0;
}

FieldsData::~FieldsData(void) {
    if(fieldsData8)
        Mjvm::free(fieldsData8);
    if(fieldsData16)
        Mjvm::free(fieldsData16);
    if(fieldsData32)
        Mjvm::free(fieldsData32);
    if(fieldsData64)
        Mjvm::free(fieldsData64);
    if(fieldsObject)
        Mjvm::free(fieldsObject);
}

ClassData::ClassData(const ClassLoader &classLoader, FieldsData *filedsData) : classLoader(classLoader), filedsData(filedsData) {

}

ClassData::~ClassData() {
    if(filedsData)
        Mjvm::free(filedsData);
}

ClassDataNode::ClassDataNode(const ClassLoader &classLoader, FieldsData *filedsData) : ClassData(classLoader, filedsData) {
    ownId = 0;
    monitorCount = 0;
    next = 0;
}
