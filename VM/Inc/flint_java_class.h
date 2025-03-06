
#ifndef __FLINT_JAVA_CLASS_H
#define __FLINT_JAVA_CLASS_H

#include "flint_java_string.h"

class FlintJavaClass : public FlintJavaObject {
public:
    FlintJavaString &getName(void) const;
    void setName(FlintJavaString *name);

    bool isArray(void) const;
    bool isPrimitive(void) const;
    const FlintConstUtf8 &getBaseTypeName(class Flint &flint, uint32_t *dimensions = 0) const;
protected:
    FlintJavaClass(void) = delete;
    FlintJavaClass(const FlintJavaClass &) = delete;
    void operator=(const FlintJavaClass &) = delete;
};

class FlintConstClass {
private:
    FlintConstClass *next;
public:
    FlintJavaClass &flintClass;
private:
    FlintConstClass(FlintJavaClass &flintClass);
    FlintConstClass(const FlintConstClass &) = delete;
    void operator=(const FlintConstClass &) = delete;

    friend class Flint;
};

#endif /* __FLINT_JAVA_CLASS_H */
