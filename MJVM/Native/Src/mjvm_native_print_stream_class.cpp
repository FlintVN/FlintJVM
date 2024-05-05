
#include <stdio.h>
#include "mjvm_string.h"
#include "mjvm_const_name.h"
#include "mjvm_native_print_stream_class.h"

static bool nativeWrite(Execution &execution) {
    MjvmString *str = (MjvmString *)execution.stackPopObject();
    if(str == 0) {
        putchar('n');
        putchar('u');
        putchar('l');
        putchar('l');
    }
    else {
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
    }
    return true;
}

static bool nativeWriteln(Execution &execution) {
    if(!nativeWrite(execution))
        return false;
    putchar('\n');
    return true;
}

static const NativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00""write",   "\x15\x00""(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("\x07\x00""writeln", "\x15\x00""(Ljava/lang/String;)V", nativeWriteln),
};

const NativeClass PRINT_STREAM_CLASS = NATIVE_CLASS(printStreamClass, methods);
