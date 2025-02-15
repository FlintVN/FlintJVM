
#ifndef __FLINT_JAVA_THREAD_H
#define __FLINT_JAVA_THREAD_H

#include "flint_java_object.h"

class FlintJavaThread : public FlintJavaObject {
public:
    FlintJavaObject *getTask(void) const;
protected:
    FlintJavaThread(void) = delete;
    FlintJavaThread(const FlintJavaThread &) = delete;
    void operator=(const FlintJavaThread &) = delete;
};

#endif /* __FLINT_JAVA_THREAD_H */
