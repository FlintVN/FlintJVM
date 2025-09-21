
#include "flint_mutex.h"
#include "flint_system_api.h"

FMutex::FMutex(void) {
    locked.clear();
    lockNest = 0;
    lockThread = NULL;
}

void FMutex::lock(void) {
    void *currentThread = FlintAPI::Thread::getCurrentThread();
    while(1) {
        while(atomic_flag_test_and_set_explicit(&locked, memory_order_acquire))
            FlintAPI::Thread::sleep(1);
        if(lockThread == NULL) {
            lockNest = 1;
            lockThread = currentThread;
            atomic_flag_clear_explicit(&locked, memory_order_release);
            return;
        }
        else if(lockThread == currentThread) {
            lockNest++;
            atomic_flag_clear_explicit(&locked, memory_order_release);
            return;
        }
        atomic_flag_clear_explicit(&locked, memory_order_release);
        FlintAPI::Thread::yield();
    }
}

void FMutex::unlock(void) {
    while(atomic_flag_test_and_set_explicit(&locked, memory_order_acquire))
        FlintAPI::Thread::sleep(1);
    if(lockNest > 0) {
        if(--lockNest == 0)
            lockThread = NULL;
    }
    atomic_flag_clear_explicit(&locked, memory_order_release);
}
