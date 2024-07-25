
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

void *FlintSystem_ThreadCreate(void (*task)(void *), void *param, uint32_t stackSize) {
    throw "FlintSystem_ThreadCreate is not implemented in VM";
}

void FlintSystem_ThreadTerminate(void *threadHandle) {
    throw "FlintSystem_ThreadTerminate is not implemented in VM";
}

void FlintSystem_ThreadSleep(uint32_t ms) {
    throw "FlintSystem_ThreadSleep is not implemented in VM";
}
