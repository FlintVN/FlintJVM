
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

ThreadHandle FlintAPI::Thread::create(void (*task)(void *), void *param, uint32_t stackSize) {
    #error "FlintAPI::Thread::create is not implemented in VM";
}

ThreadHandle FlintAPI::Thread::getCurrentThread(void) {
    #error "FlintAPI::Thread::getCurrentThread is not implemented in VM";
}

void FlintAPI::Thread::terminate(ThreadHandle handle) {
    #error "FlintAPI::Thread::terminate is not implemented in VM";
}

void FlintAPI::Thread::sleep(uint32_t ms) {
    #error "FlintAPI::Thread::sleep is not implemented in VM";
}

void FlintAPI::Thread::yield(void) {
    #error "FlintAPI::Thread::yield is not implemented in VM";
}
