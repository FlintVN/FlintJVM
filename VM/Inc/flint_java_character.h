
#ifndef __FLINT_JAVA_CHARACTER_H
#define __FLINT_JAVA_CHARACTER_H

#include "flint_java_object.h"

class FlintJavaCharacter : public FlintJavaObject {
public:
    uint16_t getValue(void) const;
    void setValue(uint16_t value);
protected:
    FlintJavaCharacter(void) = delete;
    FlintJavaCharacter(const FlintJavaCharacter &) = delete;
    void operator=(const FlintJavaCharacter &) = delete;
};

#endif /* __FLINT_JAVA_CHARACTER_H */
