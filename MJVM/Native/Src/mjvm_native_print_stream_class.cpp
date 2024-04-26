
#include <stdio.h>
#include "mjvm_string.h"
#include "mjvm_const_name.h"
#include "mjvm_native_print_stream_class.h"

static int64_t nativeWrite(int32_t args[], int32_t argc) {
    MjvmString *str = (MjvmString *)args[0];
    const char *value = str->getText();
    uint32_t length = str->getLength();
    if(str->getCoder() == 0) {
        for(uint32_t i = 0; i < length; i++)
            putchar(value[i]);
    }
    else {
        for(uint32_t i = 0; i < length; i++)
            putchar(((uint16_t *)value)[i]);
    }
    return 0;
}

static int64_t nativeWriteln(int32_t args[], int32_t argc) {
    nativeWrite(args, argc);
    putchar('\n');
    return 0;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00""write",   "\x15\x00""(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("\x07\x00""writeln", "\x15\x00""(Ljava/lang/String;)V", nativeWriteln),
};

const NativeClass PRINT_STREAM_CLASS = NATIVE_CLASS(printStreamClass, methods);
