
#include "flint_string.h"
#include "flint_const_name.h"
#include "flint_system_api.h"
#include "flint_native_print_stream_class.h"

static bool nativeWrite(FlintExecution &execution) {
    FlintString *str = (FlintString *)execution.stackPopObject();
    if(str == 0)
        FlintAPI::System::print("null", 4, 0);
    else
        FlintAPI::System::print(str->getText(), str->getLength(), str->getCoder());
    return true;
}

static bool nativeWriteln(FlintExecution &execution) {
    if(!nativeWrite(execution))
        return false;
    FlintAPI::System::print("\n", 1, 0);
    return true;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x05\x00\x2B\x02""write",   "\x15\x00\x47\x07""(Ljava/lang/String;)V", nativeWrite),
    NATIVE_METHOD("\x07\x00\x05\x03""writeln", "\x15\x00\x47\x07""(Ljava/lang/String;)V", nativeWriteln),
};

const FlintNativeClass PRINT_STREAM_CLASS = NATIVE_CLASS(printStreamClassName, methods);
