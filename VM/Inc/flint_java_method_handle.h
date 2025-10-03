
#ifndef __FLINT_JAVA_METHOD_HANDLE_H
#define __FLINT_JAVA_METHOD_HANDLE_H

#include "flint_java_object.h"
#include "flint_java_class.h"

class JMethodHandle : public JObject {
public:
    JObject *getMethodType(void) const;

    MethodInfo *getTargetMethod(void) const;
    const char *getClassName(void) const;
    const char *getName(void) const;
    const char *getDesc(void) const;
    uint8_t getArgc(void) const;
    RefKind getRefKind(void) const;

    void setTargetMethod(MethodInfo *methodInfo);
private:
    JMethodHandle(void) = delete;
    JMethodHandle(const JMethodHandle &) = delete;
    void operator=(const JMethodHandle &) = delete;

    void setMethodType(JObject *methodType);
    void setTarget(const char *clsName, const char *name, const char *desc, RefKind refKind);

    static uint32_t size(void);

    friend class Flint;
};

#endif /* __FLINT_JAVA_METHOD_HANDLE_H */
