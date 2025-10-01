
#ifndef __FLINT_JAVA_METHOD_HANDLE_H
#define __FLINT_JAVA_METHOD_HANDLE_H

#include "flint_java_object.h"

class JMethodHandle : public JObject {
public:
    JObject *getMethodType(void) const;

    ConstMethod *getConstMethod(void) const;
    RefKind getRefKind(void) const;
private:
    JMethodHandle(void) = delete;
    JMethodHandle(const JMethodHandle &) = delete;
    void operator=(const JMethodHandle &) = delete;

    void setMethodType(JObject *methodType);
    void setConstMethod(ConstMethod *constMethod);
    void setRefKind(RefKind refKind);

    static uint32_t size(void);

    friend class Flint;
};

#endif /* __FLINT_JAVA_METHOD_HANDLE_H */
