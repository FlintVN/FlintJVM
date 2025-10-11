
#include <new>
#include <string.h>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_java_object.h"

JObject::JObject(uint32_t size, JClass *type) :
ListNode(),
size(size), prot(0x02), type(type), monitorCount(0), ownId(0) {

}

const char *JObject::getTypeName(void) const {
    if(type == NULL)
        return "java/lang/Class";
    return type->getTypeName();
}

bool JObject::initFields(FExec *ctx, ClassLoader *loader) {
    FieldsData *field = (FieldsData *)data;
    new (field)FieldsData();

    return field->init(ctx, loader, false);
}

static void throwNoSuchFieldError(FExec *ctx, const char *clsName, const char *name) {
    JClass *excpCls = Flint::findClass(ctx, "java/lang/NoSuchFieldError");
    ctx->throwNew(excpCls, "Could not find the field %s.%s", clsName, name);
}

FieldValue *JObject::getField(FExec *ctx, ConstField *field) const {
    FieldValue *ret = ((FieldsData *)data)->getField(field);
    if(ret == NULL && ctx != NULL)
        throwNoSuchFieldError(ctx, field->className, field->nameAndType->name);
    return ret;
}

FieldValue *JObject::getField(FExec *ctx, const char *name) const {
    FieldValue *ret = ((FieldsData *)data)->getField(name);
    if(ret == NULL && ctx != NULL)
        throwNoSuchFieldError(ctx, getTypeName(), name);
    return ret;
}

FieldValue *JObject::getFieldByIndex(uint32_t index) const {
    return ((FieldsData *)data)->getFieldByIndex(index);
}

void JObject::clearData(void) {
    memset(data, 0, size);
}

bool JObject::isArray(void) const {
    return (type != NULL && type->isArray());
}

void JObject::clearProtected(void) {
    prot = 0;
}

void JObject::setProtected(void) {
    prot = 1;
}

uint8_t JObject::getProtected(void) const {
    return prot;
}

JObject::~JObject(void) {
    if(type == NULL || type->isArray() == false)
        ((FieldsData *)data)->~FieldsData();
}
