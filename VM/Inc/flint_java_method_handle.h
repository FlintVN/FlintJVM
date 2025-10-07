
#ifndef __FLINT_JAVA_METHOD_HANDLE_H
#define __FLINT_JAVA_METHOD_HANDLE_H

#include "flint_java_object.h"
#include "flint_java_class.h"

class JMethodHandle : public JObject {
public:
    JObject *getMethodType(void) const;

    MethodInfo *getTargetMethod(class FExec *ctx, JClass *caller = NULL) const;
    const char *getTargetClassName(void) const;
    const char *getTargetName(void) const;
    const char *getTargetDesc(void) const;
    uint8_t getTargetArgc(void) const;
    RefKind getTargetRefKind(void) const;
private:
    JMethodHandle(void) = delete;
    JMethodHandle(const JMethodHandle &) = delete;
    void operator=(const JMethodHandle &) = delete;

    void setMethodType(JObject *methodType);
    void setTarget(JMethodHandle *methodHandle);
    void setTarget(const char *clsName, const char *name, const char *desc, RefKind refKind);

    static uint32_t size(void);

    friend class Flint;
};

#endif /* __FLINT_JAVA_METHOD_HANDLE_H */
