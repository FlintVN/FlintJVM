
#include "flint_java_method_handle.h"

typedef struct {
    FieldsData fields;
    ConstMethod *constMethod;
    RefKind refKind;
} InternalData;

JObject *JMethodHandle::getMethodType(void) const {
    return getFieldObjByIndex(0)->value;
}

void JMethodHandle::setMethodType(JObject *methodType) {
    getFieldObjByIndex(0)->value = methodType;
}

ConstMethod *JMethodHandle::getConstMethod(void) const {
    return ((InternalData *)data)->constMethod;
}

void JMethodHandle::setConstMethod(ConstMethod *constMethod) {
    ((InternalData *)data)->constMethod = constMethod;
}

RefKind JMethodHandle::getRefKind(void) const {
    return ((InternalData *)data)->refKind;
}

void JMethodHandle::setRefKind(RefKind refKind) {
    ((InternalData *)data)->refKind = refKind;
}

uint32_t JMethodHandle::size(void) {
    return sizeof(JMethodHandle) + sizeof(InternalData);
}
