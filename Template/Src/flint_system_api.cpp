
#error "Do not build this file. You need to create a new file based on this template file and implement the functions defined in this file."

#include "flint_system_api.h"

void *FlintAPI::System::malloc(uint32_t size) {
    throw "FlintAPI::System::malloc is not implemented in VM";
}

void *FlintAPI::System::realloc(void *p, uint32_t size) {
    throw "FlintAPI::System::realloc is not implemented in VM";
}

void FlintAPI::System::free(void *p) {
    throw "FlintAPI::System::free is not implemented in VM";
}

void FlintAPI::System::print(const char *text, uint32_t length, uint8_t coder) {
    throw "FlintAPI::System::print is not implemented in VM";
}

int64_t FlintAPI::System::getNanoTime(void) {
    throw "FlintAPI::System::getNanoTime is not implemented in VM";
}
