
#ifndef __FLINT_JAVA_CHAR_H
#define __FLINT_JAVA_CHAR_H

#include "flint_java_object.h"

class FlintJavaChar : public FlintJavaObject {
public:
    uint16_t getValue(void) const;
    void setValue(uint16_t value);
protected:
    FlintJavaChar(void) = delete;
    FlintJavaChar(const FlintJavaChar &) = delete;
    void operator=(const FlintJavaChar &) = delete;
};

#endif /* __FLINT_JAVA_CHAR_H */
