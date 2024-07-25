
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "mjvm_system_api.h"

void *MjvmSystem_ThreadCreate(void (*task)(void *), void *param, uint32_t stackSize) {
    throw "MjvmSystem_ThreadCreate is not implemented in VM";
}

void MjvmSystem_ThreadTerminate(void *threadHandle) {
    throw "MjvmSystem_ThreadTerminate is not implemented in VM";
}

void MjvmSystem_ThreadSleep(uint32_t ms) {
    throw "MjvmSystem_ThreadSleep is not implemented in VM";
}
