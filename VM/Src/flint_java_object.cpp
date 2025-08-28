
#include <new>
#include "flint.h"
#include "flint_java_class.h"
#include "flint_java_object.h"

JObject::JObject(uint32_t size, JClass *type) :
ListNode<JObject>(),
size(size), prot(0x02), type(type), monitorCount(0), ownId(0) {

}

const char *JObject::getTypeName(void) const {
    if(type == NULL)
        return "java/lang/Class";
    return type->getTypeName();
}

FieldsData *JObject::getFields(void) const {
    return (FieldsData *)data;
}

bool JObject::initFields(FExec *ctx, ClassLoader *loader) {
    FieldsData *field = getFields();
    new (field)FieldsData();

    return field->init(ctx, loader, false);
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
        getFields()->~FieldsData();
}
