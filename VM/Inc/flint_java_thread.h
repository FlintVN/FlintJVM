
#ifndef __FLINT_JAVA_THREAD_H
#define __FLINT_JAVA_THREAD_H

#include "flint_java_object.h"

class JThread : public JObject {
public:
    JObject *getTask(void) const;
protected:
    JThread(void) = delete;
    JThread(const JThread &) = delete;
    void operator=(const JThread &) = delete;
};

#endif /* __FLINT_JAVA_THREAD_H */
