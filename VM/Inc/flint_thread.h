
#ifndef __FLINT_THREAD_H
#define __FLINT_THREAD_H

#include "flint_object.h"

class FlintThread : public FlintObject {
public:
    FlintObject *getTask(void) const;
protected:
    FlintThread(void) = delete;
    FlintThread(const FlintThread &) = delete;
    void operator=(const FlintThread &) = delete;
};

#endif /* __FLINT_THREAD_H */
