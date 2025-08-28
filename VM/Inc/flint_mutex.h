
#ifndef __FLINT_MUTEX_H
#define __FLINT_MUTEX_H

#include <atomic>
#include "flint_std.h"

using namespace std;

class FMutex {
private:
    atomic_flag locked;
    volatile uint32_t lockNest;
    volatile void *lockThread;

    FMutex(const FMutex &) = delete;
    void operator=(const FMutex &) = delete;
public:
    FMutex(void);

    void lock(void);
    void unlock(void);
};

#endif /* __FLINT_MUTEX_H */
