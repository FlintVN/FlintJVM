
#include <string.h>
#include "flint_method_info.h"
#include "flint_system_api.h"
#include "flint_class_loader.h"
#include "flint_native.h"
#include "flint_native_math.h"
#include "flint_native_class.h"
#include "flint_native_array.h"
#include "flint_native_float.h"
#include "flint_native_double.h"
#include "flint_native_object.h"
#include "flint_native_string.h"
#include "flint_native_system.h"
#include "flint_native_thread.h"
#include "flint_native_character.h"
#include "flint_native_print_stream.h"

static constexpr NativeClass BASE_NATIVE_CLASS_LIST[] = {
    NATIVE_CLASS("java/lang/Math",          mathMethods),
    NATIVE_CLASS("java/lang/Class",         classMethods),
    NATIVE_CLASS("java/lang/reflect/Array", arrayMethods),
    NATIVE_CLASS("java/lang/Float",         floatMethods),
    NATIVE_CLASS("java/lang/Double",        doubleMethods),
    NATIVE_CLASS("java/lang/Object",        objectMethods),
    NATIVE_CLASS("java/lang/String",        stringMethods),
    NATIVE_CLASS("java/lang/System",        systemMethods),
    NATIVE_CLASS("java/lang/Thread",        threadMethods),
    NATIVE_CLASS("java/lang/Character",     characterMethods),
    NATIVE_CLASS("java/io/PrintStream",     printStreamMethods),
};

JNMPtr NativeClass::findNativeMethod(MethodInfo *methodInfo) {
    uint32_t classNameHash = methodInfo->loader->getHashKey();
    for(uint32_t i = 0; i < LENGTH(BASE_NATIVE_CLASS_LIST); i++) {
        const NativeClass *nativeCls = &BASE_NATIVE_CLASS_LIST[i];
        if(
            classNameHash == nativeCls->hash &&
            strcmp(nativeCls->className, methodInfo->loader->getName()) == 0
        ) {
            for(uint32_t k = 0; k < nativeCls->methodCount; k++) {
                if(
                    nativeCls->methods[k].hash == methodInfo->hash &&
                    strcmp(nativeCls->methods[k].name, methodInfo->name) == 0 &&
                    strcmp(nativeCls->methods[k].desc, methodInfo->desc) == 0
                ) {
                    return (JNMPtr)nativeCls->methods[k].methodPtr;
                }
            }
            break;
        }
    }
    return FlintAPI::System::findNativeMethod(methodInfo);
}
