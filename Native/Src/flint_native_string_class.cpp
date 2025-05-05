
#include "flint.h"
#include "flint_java_object.h"
#include "flint_const_name_base.h"
#include "flint_native_string_class.h"

static FlintError nativeIntern(FlintExecution &execution) {
    FlintJavaString *obj = (FlintJavaString *)execution.stackPopObject();
    FlintJavaString *strIntern;
    RETURN_IF_ERR(execution.flint.getConstString(*obj, strIntern));
    execution.stackPushObject(strIntern);
    return ERR_OK;
}

static const FlintNativeMethod methods[] = {
    NATIVE_METHOD("\x06\x00\xB1\xB7""intern", "\x14\x00\xA7\xAF""()Ljava/lang/String;", nativeIntern),
};

const FlintNativeClass STRING_CLASS = NATIVE_CLASS(stringClassName, methods);
