
#include "flint_java_method_handle.h"

typedef struct {
    FieldsData fields;
    MethodInfo *methodInfo;
    const char *clsName;
    const char *name;
    const char *desc;
    uint8_t argc;
    RefKind refKind;
} InternalData;

JObject *JMethodHandle::getMethodType(void) const {
    return getFieldObjByIndex(0)->value;
}

void JMethodHandle::setMethodType(JObject *methodType) {
    getFieldObjByIndex(0)->value = methodType;
}

MethodInfo *JMethodHandle::getTargetMethod(void) const {
    return ((InternalData *)data)->methodInfo;
}

const char *JMethodHandle::getClassName(void) const {
    return ((InternalData *)data)->clsName;
}

const char *JMethodHandle::getName(void) const {
    return ((InternalData *)data)->name;
}

const char *JMethodHandle::getDesc(void) const {
    return ((InternalData *)data)->desc;
}

uint8_t JMethodHandle::getArgc(void) const {
    return ((InternalData *)data)->argc;
}

RefKind JMethodHandle::getRefKind(void) const {
    return ((InternalData *)data)->refKind;
}

void JMethodHandle::setTargetMethod(MethodInfo *methodInfo) {
    ((InternalData *)data)->methodInfo = methodInfo;
}

void JMethodHandle::setTarget(const char *clsName, const char *name, const char *desc, RefKind refKind) {
    extern uint8_t parseArgc(const char *desc);
    ((InternalData *)data)->methodInfo = NULL;
    ((InternalData *)data)->clsName = clsName;
    ((InternalData *)data)->name = name;
    ((InternalData *)data)->desc = desc;
    ((InternalData *)data)->argc = parseArgc(desc);
    ((InternalData *)data)->refKind = refKind;
}

uint32_t JMethodHandle::size(void) {
    return sizeof(JMethodHandle) + sizeof(InternalData);
}
