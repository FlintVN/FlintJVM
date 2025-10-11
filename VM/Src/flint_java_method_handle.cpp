
#include "flint.h"
#include "flint_execution.h"
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
    return getFieldByIndex(0)->getObj();
}

void JMethodHandle::setMethodType(JObject *methodType) {
    getFieldByIndex(0)->setObj(methodType);
}

MethodInfo *JMethodHandle::getTargetMethod(FExec *ctx, JClass *caller) const {
    InternalData *internalData = (InternalData *)data;
    MethodInfo *methodInfo = internalData->methodInfo;
    if(caller != NULL && internalData->refKind == REF_INVOKEVIRTUAL) {
        if(methodInfo == NULL || methodInfo->loader != caller->getClassLoader()) {
            methodInfo = Flint::findMethod(ctx, caller, internalData->name, internalData->desc);
            if(methodInfo == NULL) return NULL;
            ((InternalData *)data)->methodInfo = methodInfo;
        }
    }
    else if(methodInfo == NULL) {
        JClass *targetCls = Flint::findClass(ctx, internalData->clsName);
        methodInfo = Flint::findMethod(ctx, targetCls, internalData->name, internalData->desc);
        if(methodInfo == NULL) return NULL;
        ((InternalData *)data)->methodInfo = methodInfo;
    }
    return methodInfo;
}

const char *JMethodHandle::getTargetClassName(void) const {
    return ((InternalData *)data)->clsName;
}

const char *JMethodHandle::getTargetName(void) const {
    return ((InternalData *)data)->name;
}

const char *JMethodHandle::getTargetDesc(void) const {
    return ((InternalData *)data)->desc;
}

uint8_t JMethodHandle::getTargetArgc(void) const {
    return ((InternalData *)data)->argc;
}

RefKind JMethodHandle::getTargetRefKind(void) const {
    return ((InternalData *)data)->refKind;
}

void JMethodHandle::setTarget(JMethodHandle *methodHandle) {
    InternalData *src = (InternalData *)methodHandle->data;
    InternalData *desc = (InternalData *)data;
    desc->methodInfo = NULL;
    desc->clsName = src->clsName;
    desc->name = src->name;
    desc->desc = src->desc;
    desc->argc = src->argc;
    desc->refKind = src->refKind;
}

void JMethodHandle::setTarget(const char *clsName, const char *name, const char *desc, RefKind refKind) {
    ((InternalData *)data)->methodInfo = NULL;
    ((InternalData *)data)->clsName = clsName;
    ((InternalData *)data)->name = name;
    ((InternalData *)data)->desc = desc;
    ((InternalData *)data)->argc = getArgSlotCount(desc);
    ((InternalData *)data)->refKind = refKind;
}

uint32_t JMethodHandle::size(void) {
    return sizeof(JMethodHandle) + sizeof(InternalData);
}
