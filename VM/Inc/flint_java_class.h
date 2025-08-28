
#ifndef __FLINT_JAVA_CLASS_H
#define __FLINT_JAVA_CLASS_H

#include "flint_java_object.h"
#include "flint_class_loader.h"

class JClass : public JObject {
public:
    const char *getTypeName(void) const;
    ClassLoader *getClassLoader(void) const;

    static char isPrimitive(const char *typeName, uint16_t length = 0xFFFF);

    bool isPrimitive(void) const;
    bool isArray(void) const;

    uint8_t componentSize() const;
private:
    JClass(const char *typeName, ClassLoader *loader);
    JClass(const JClass &) = delete;
    void operator=(const JClass &) = delete;

    static uint32_t size(void);

    friend class Flint;
};

#endif /* __FLINT_JAVA_CLASS_H */
