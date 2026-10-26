
#include "flint_mutex.h"
#include "flint_system_api.h"

FMutex::FMutex(void) {
    locked.store(false, memory_order_relaxed);
    lockNest = 0;
    lockThread = NULL;
}

void FMutex::lock(void) {
    void *currentThread = FlintAPI::Thread::getCurrentThread();
    while (1) {
        while (locked.exchange(true, memory_order_acquire))
            FlintAPI::Thread::yield();
        if (lockThread == NULL) {
            lockNest = 1;
            lockThread = currentThread;
            locked.exchange(false, memory_order_release);
            return;
        }
        else if (lockThread == currentThread) {
            lockNest++;
            locked.exchange(false, memory_order_release);
            return;
        }
        locked.exchange(false, memory_order_release);
        FlintAPI::Thread::yield();
    }
}

void FMutex::unlock(void) {
    void *currentThread = FlintAPI::Thread::getCurrentThread();
    while (locked.exchange(true, memory_order_acquire))
        FlintAPI::Thread::yield();
    if (currentThread == lockThread && lockNest > 0) {
        if (--lockNest == 0)
            lockThread = NULL;
    }
    locked.exchange(false, memory_order_release);
}
