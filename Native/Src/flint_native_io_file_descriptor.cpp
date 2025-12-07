
#include "Flint.h"
#include "flint_common.h"
#include "flint_native_io_file_descriptor.h"

jvoid NativeFileDescriptor_Sync(FNIEnv *env, jobject obj) {
    int32_t fd = obj->getFieldByIndex(0)->getInt32();
    if(fd < 0)
        env->throwNew(env->findClass("java/io/SyncFailedException"), "sync failed");
    else {
        // TODO
    }
}
