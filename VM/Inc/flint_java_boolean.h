
#ifndef __FLINT_JAVA_BOOLEAN_H
#define __FLINT_JAVA_BOOLEAN_H

#include "flint_java_object.h"

class FlintJavaBoolean : public JObject {
public:
    bool getValue(void) const;
    void setValue(bool value);
protected:
    FlintJavaBoolean(void) = delete;
    FlintJavaBoolean(const FlintJavaBoolean &) = delete;
    void operator=(const FlintJavaBoolean &) = delete;
};

#endif /* __FLINT_JAVA_BOOLEAN_H */
