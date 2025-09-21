
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_java_object.h"
#include "flint_class_loader.h"
#include "flint_fields_data.h"

Field32::Field32(const FieldInfo *fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

Field64::Field64(const FieldInfo *fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

FieldObj::FieldObj(const FieldInfo *fieldInfo) : fieldInfo(fieldInfo), value(NULL) {

}

FieldsData::FieldsData(void) :
fields32Count(0), fields64Count(0), fieldsObjCount(0),
fields32(NULL), fields64(NULL), fieldsObj(NULL) {

}

bool FieldsData::init(class FExec *ctx, class ClassLoader *loader, bool isStatic) {
    bool ret = isStatic ? initStatic(ctx, loader) : initNonStatic(ctx, loader);
    if(ret == false)
        this->~FieldsData();
    return ret;
}

bool FieldsData::initStatic(FExec *ctx, ClassLoader *loader) {
    uint16_t fieldsCount = loader->getFieldsCount();
    uint16_t field32Index = 0;
    uint16_t field64Index = 0;
    uint16_t fieldObjIndex = 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        FieldInfo *fieldInfo = loader->getFieldInfo(index);
        if((fieldInfo->accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo->desc[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    fields64Count++;
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    fieldsObjCount++;
                    break;
                default:
                    fields32Count++;
                    break;
            }
        }
    }

    if(fields32Count) {
        fields32 = (Field32 *)Flint::malloc(ctx, fields32Count * sizeof(Field32));
        if(fields32 == NULL) return false;
    }

    if(fields64Count) {
        fields64 = (Field64 *)Flint::malloc(ctx, fields64Count * sizeof(Field64));
        if(fields64 == NULL) return false;
    }

    if(fieldsObjCount) {
        fieldsObj = (FieldObj *)Flint::malloc(ctx, fieldsObjCount * sizeof(FieldObj));
        if(fieldsObj == NULL) return false;
    }

    for(uint16_t index = 0; index < fieldsCount; index++) {
        FieldInfo *fieldInfo = loader->getFieldInfo(index);
        if((fieldInfo->accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo->desc[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    new (&fields64[field64Index++])Field64(fieldInfo);
                    break;
                case 'L':   /* An instance of class ClassName */
                case '[':   /* Array */
                    new (&fieldsObj[fieldObjIndex++])FieldObj(fieldInfo);
                    break;
                default:
                    new (&fields32[field32Index++])Field32(fieldInfo);
                    break;
            }
        }
    }

    return true;
}

bool FieldsData::initNonStatic(FExec *ctx, ClassLoader *loader) {
    ClassLoader *ld = loader;

    while(ld) {
        uint16_t fieldsCount = ld->getFieldsCount();
        for(uint16_t index = 0; index < fieldsCount; index++) {
            FieldInfo *fieldInfo = ld->getFieldInfo(index);
            if((fieldInfo->accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo->desc[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        fields64Count++;
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        fieldsObjCount++;
                        break;
                    default:
                        fields32Count++;
                        break;
                }
            }
        }
        /* Don't use ld->getSuperClass here to avoid endless recursion */
        const char *superName = ld->getSuperClassName();
        if(superName == NULL) break;
        ld = Flint::findLoader(ctx, superName);
        if(ld == NULL) return false;
    }

    if(fields32Count) {
        fields32 = (Field32 *)Flint::malloc(ctx, fields32Count * sizeof(Field32));
        if(fields32 == NULL) return false;
    }

    if(fields64Count) {
        fields64 = (Field64 *)Flint::malloc(ctx, fields64Count * sizeof(Field64));
        if(fields64 == NULL) return false;
    }

    if(fieldsObjCount) {
        fieldsObj = (FieldObj *)Flint::malloc(ctx, fieldsObjCount * sizeof(FieldObj));
        if(fieldsObj == NULL) return false;
    }

    uint16_t field32Index = fields32Count;
    uint16_t field64Index = fields64Count;
    uint16_t fieldObjIndex = fieldsObjCount;

    ld = loader;
    while(ld) {
        uint16_t fieldsCount = ld->getFieldsCount();
        for(int16_t index = fieldsCount - 1; index >= 0; index--) {
            FieldInfo *fieldInfo = ld->getFieldInfo(index);
            if((fieldInfo->accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo->desc[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fields64[--field64Index])Field64(fieldInfo);
                        break;
                    case 'L':   /* An instance of class ClassName */
                    case '[':   /* Array */
                        new (&fieldsObj[--fieldObjIndex])FieldObj(fieldInfo);
                        break;
                    default:
                        new (&fields32[--field32Index])Field32(fieldInfo);
                        break;
                }
            }
        }
        /* Don't use ld->getSuperClass here to avoid endless recursion */
        const char *superName = ld->getSuperClassName();
        if(superName == NULL) break;
        ld = Flint::findLoader(ctx, superName);
        if(ld == NULL) return false;
    }

    return true;
}

Field32 *FieldsData::getField32(ConstField *field) {
    if(field->fieldIndex == 0 && fields32Count) {
        for(uint16_t i = 0; i < fields32Count; i++) {
            if(
                field->nameAndType->hash == fields32[i].fieldInfo->hash &&
                strcmp(field->nameAndType->name, fields32[i].fieldInfo->name) == 0 &&
                strcmp(field->nameAndType->desc, fields32[i].fieldInfo->desc) == 0
            ) {
                field->fieldIndex = i | 0x80000000;
                break;
            }
        }
        if(field->fieldIndex == 0) return NULL;
    }
    return &fields32[field->fieldIndex & 0x7FFFFFFF];
}

Field32 *FieldsData::getField32(const char *name) {
    if(fields32Count) {
        uint16_t hash = Hash(name);
        for(uint16_t i = 0; i < fields32Count; i++) {
            const FieldInfo *fieldInfo = fields32[i].fieldInfo;
            if(hash == (uint16_t)fieldInfo->hash && strcmp(name, fieldInfo->name) == 0)
                return &fields32[i];
        }
    }
    return NULL;
}

Field32 *FieldsData::getField32ByIndex(uint32_t index) {
    return &fields32[index];
}

Field64 *FieldsData::getField64(ConstField *field) {
    if(field->fieldIndex == 0 && fields64Count) {
        for(uint16_t i = 0; i < fields64Count; i++) {
            if(
                field->nameAndType->hash == fields64[i].fieldInfo->hash &&
                strcmp(field->nameAndType->name, fields64[i].fieldInfo->name) == 0 &&
                strcmp(field->nameAndType->desc, fields64[i].fieldInfo->desc) == 0
            ) {
                field->fieldIndex = i | 0x80000000;
                break;
            }
        }
        if(field->fieldIndex == 0) return NULL;
    }
    return &fields64[field->fieldIndex & 0x7FFFFFFF];
}

Field64 *FieldsData::getField64(const char *name) {
    if(fields64Count) {
        uint16_t hash = Hash(name);
        for(uint16_t i = 0; i < fields64Count; i++) {
            const FieldInfo *fieldInfo = fields64[i].fieldInfo;
            if(hash == (uint16_t)fieldInfo->hash && strcmp(name, fieldInfo->name) == 0)
                return &fields64[i];
        }
    }
    return NULL;
}

Field64 *FieldsData::getField64ByIndex(uint32_t index) {
    return &fields64[index];
}

FieldObj *FieldsData::getFieldObj(ConstField *field) {
    if(field->fieldIndex == 0 && fieldsObjCount) {
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            if(
                field->nameAndType->hash == fieldsObj[i].fieldInfo->hash &&
                strcmp(field->nameAndType->name, fieldsObj[i].fieldInfo->name) == 0 &&
                strcmp(field->nameAndType->desc, fieldsObj[i].fieldInfo->desc) == 0
            ) {
                field->fieldIndex = i | 0x80000000;
                break;
            }
        }
        if(field->fieldIndex == 0) return NULL;
    }
    return &fieldsObj[field->fieldIndex & 0x7FFFFFFF];
}

FieldObj *FieldsData::getFieldObj(const char *name) {
    if(fieldsObjCount) {
        uint16_t hash = Hash(name);
        for(uint16_t i = 0; i < fieldsObjCount; i++) {
            const FieldInfo *fieldInfo = fieldsObj[i].fieldInfo;
            if(hash == (uint16_t)fieldInfo->hash && strcmp(name, fieldInfo->name) == 0)
                return &fieldsObj[i];
        }
    }
    return NULL;
}

FieldObj *FieldsData::getFieldObjByIndex(uint32_t index) {
    return &fieldsObj[index];
}

FieldsData::~FieldsData(void) {
    if(fields32) Flint::free(fields32);
    if(fields64) Flint::free(fields64);
    if(fieldsObj) Flint::free(fieldsObj);
}
