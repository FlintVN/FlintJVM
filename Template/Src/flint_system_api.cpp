
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint.h"
#include "flint_system_api.h"

void FlintAPI::System::reset(Flint &flint) {
    #error "FlintAPI::System::reset is not implemented in VM"
}

void *FlintAPI::System::malloc(uint32_t size) {
    #error "FlintAPI::System::malloc is not implemented in VM"
}

void *FlintAPI::System::realloc(void *p, uint32_t size) {
    #error "FlintAPI::System::realloc is not implemented in VM"
}

void FlintAPI::System::free(void *p) {
    #error "FlintAPI::System::free is not implemented in VM"
}

bool FlintAPI::System::isInHeapRegion(void *addr) {
    #error "FlintAPI::System::isInHeapRegion is not implemented in VM"
}

void FlintAPI::System::print(const char *text, uint32_t length, uint8_t coder) {
    #error "FlintAPI::System::print is not implemented in VM"
}

uint64_t FlintAPI::System::getNanoTime(void) {
    #error "FlintAPI::System::getNanoTime is not implemented in VM"
}

FlintNativeMethodPtr FlintAPI::System::findNativeMethod(const FlintMethodInfo &methodInfo) {
    #error "FlintAPI::System::findNativeMethod is not implemented in VM"
}
