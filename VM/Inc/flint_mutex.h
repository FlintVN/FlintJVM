
#ifndef __FLINT_MUTEX_H
#define __FLINT_MUTEX_H

#include <stdatomic.h>

class FlintMutex {
private:
    atomic_flag locked;
    volatile uint32_t lockNest;
    volatile void *lockThread;

    FlintMutex(const FlintMutex &) = delete;
    void operator=(const FlintMutex &) = delete;
public:
    FlintMutex(void);

    void lock(void);
    void unlock(void);
};

#endif /* __FLINT_MUTEX_H */
