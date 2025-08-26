
#ifndef __FLINT_JAVA_CLASS_H
#define __FLINT_JAVA_CLASS_H

#include "flint_java_string.h"

class JClass : public JObject {
public:
    JString *getName(void) const;
    void setName(JString *name);

    bool isArray(void) const;
    bool isPrimitive(void) const;
    FlintResult<FlintConstUtf8> getBaseTypeName(class Flint &flint, uint32_t *dimensions = 0) const;
protected:
    JClass(void) = delete;
    JClass(const JClass &) = delete;
    void operator=(const JClass &) = delete;
};

class FlintConstClass {
private:
    FlintConstClass *next1;
    FlintConstClass *next2;
public:
    JClass &flintClass;
private:
    FlintConstClass(JClass &flintClass);
    FlintConstClass(const FlintConstClass &) = delete;
    void operator=(const FlintConstClass &) = delete;

    friend class Flint;
};

#endif /* __FLINT_JAVA_CLASS_H */
