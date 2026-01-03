
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_java_object.h"
#include "flint_class_loader.h"
#include "flint_fields_data.h"

FieldValue::FieldValue(const FieldInfo *fieldInfo) : fieldInfo(fieldInfo), value(0) {

}

const FieldInfo *FieldValue::getFieldInfo(void) const {
    return fieldInfo;
}

int32_t FieldValue::getInt32(void) const {
    return (int32_t)value;
}

JObject *FieldValue::getObj(void) const {
    return (JObject *)value;
}

int64_t FieldValue::getInt64(void) const {
    return ((uint64_t)this[1].value << 32) | this[0].value;
}

bool FieldValue::getBool(void) const {
    return value != 0;
}

void FieldValue::setInt32(int32_t val) {
    value = (uint32_t)val;
}

void FieldValue::setObj(JObject *obj) {
    value = (uint32_t)obj;
}

void FieldValue::setInt64(int64_t val) {
    this[0].value = (uint32_t)val;
    this[1].value = (uint64_t)val >> 32;
}

FieldsData::FieldsData(void) : count(0), objCount(0), fields(NULL) {

}

bool FieldsData::init(class FExec *ctx, class ClassLoader *loader, bool isStatic) {
    bool ret = isStatic ? initStatic(ctx, loader) : initNonStatic(ctx, loader);
    if(ret == false)
        this->~FieldsData();
    return ret;
}

bool FieldsData::initStatic(FExec *ctx, ClassLoader *loader) {
    uint16_t fieldsCount = loader->getFieldsCount();
    uint16_t fieldIndex = 0;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        FieldInfo *fieldInfo = loader->getFieldInfo(index);
        if((fieldInfo->accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            switch(fieldInfo->desc[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    count += 2;
                    break;
                default:
                    count++;
                    break;
            }
        }
    }

    if(count == 0) return true;
    fields = (FieldValue *)Flint::malloc(ctx, count * sizeof(FieldValue));
    if(fields == NULL) return false;

    for(uint16_t index = 0; index < fieldsCount; index++) {
        FieldInfo *fieldInfo = loader->getFieldInfo(index);
        if((fieldInfo->accessFlag & FIELD_STATIC) == FIELD_STATIC) {
            new (&fields[fieldIndex++])FieldValue(fieldInfo);
            switch(fieldInfo->desc[0]) {
                case 'J':   /* Long */
                case 'D':   /* Double */
                    new (&fields[fieldIndex++])FieldValue(NULL);
                    break;
                case 'L':   /* Object */
                case '[':   /* Array */
                    objCount++;
                    break;
                default:
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
                        count += 2;
                        break;
                    default:
                        count++;
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

    if(count == 0) return true;
    fields = (FieldValue *)Flint::malloc(ctx, count * sizeof(FieldValue));
    if(fields == NULL) return false;

    uint16_t fieldIndex = count;

    ld = loader;
    while(ld) {
        uint16_t fieldsCount = ld->getFieldsCount();
        for(int16_t index = fieldsCount - 1; index >= 0; index--) {
            FieldInfo *fieldInfo = ld->getFieldInfo(index);
            if((fieldInfo->accessFlag & FIELD_STATIC) != FIELD_STATIC) {
                switch(fieldInfo->desc[0]) {
                    case 'J':   /* Long */
                    case 'D':   /* Double */
                        new (&fields[--fieldIndex])FieldValue(NULL);
                        break;
                    case 'L':   /* Object */
                    case '[':   /* Array */
                        objCount++;
                        break;
                    default:
                        break;
                }
                new (&fields[--fieldIndex])FieldValue(fieldInfo);
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

uint16_t FieldsData::hasObjField(void) const {
    return objCount;
}

FieldValue *FieldsData::getField(ConstField *field) const {
    if(field->fieldIndex == 0 && count) {
        for(uint16_t i = 0; i < count; i++) {
            const FieldInfo *fieldInfo = fields[i].fieldInfo;
            if(
                fieldInfo != NULL &&
                field->nameAndType->hash == fieldInfo->hash &&
                strcmp(field->nameAndType->name, fieldInfo->name) == 0 &&
                strcmp(field->nameAndType->desc, fieldInfo->desc) == 0
            ) {
                field->fieldIndex = i | 0x80000000;
                break;
            }
        }
        if(field->fieldIndex == 0) return NULL;
    }
    return &fields[field->fieldIndex & 0x7FFFFFFF];
}

FieldValue *FieldsData::getField(const char *name) const {
    if(count > 0) {
        uint16_t hash = Hash(name);
        for(uint16_t i = 0; i < count; i++) {
            const FieldInfo *fieldInfo = fields[i].fieldInfo;
            if(fieldInfo != NULL && hash == (uint16_t)fieldInfo->hash && strcmp(name, fieldInfo->name) == 0)
                return &fields[i];
        }
    }
    return NULL;
}

FieldValue *FieldsData::getFieldByIndex(uint32_t index) const {
    return &fields[index];
}

FieldsData::~FieldsData(void) {
    if(fields) Flint::free(fields);
}
