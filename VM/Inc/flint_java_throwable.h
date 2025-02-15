
#ifndef __FLINT_JAVA_THROWABLE_H
#define __FLINT_JAVA_THROWABLE_H

#include "flint_java_string.h"

class FlintJavaThrowable : public FlintJavaObject {
public:
    FlintJavaString *getDetailMessage(void) const;
    void setDetailMessage(FlintJavaString &strObj);
protected:
    FlintJavaThrowable(void) = delete;
    FlintJavaThrowable(const FlintJavaThrowable &) = delete;
    void operator=(const FlintJavaThrowable &) = delete;
};

#endif /* __FLINT_JAVA_THROWABLE_H */
