
#ifndef __FLINT_NATIVE_IO_FILE_DESCRIPTOR_H
#define __FLINT_NATIVE_IO_FILE_DESCRIPTOR_H

#include "flint_native.h"

jvoid NativeFileDescriptor_Sync(FNIEnv *env, jobject obj);

static constexpr NativeMethod fileDescriptorMethods[] = {
    NATIVE_METHOD("sync", "()V", NativeFileDescriptor_Sync),
};

#endif /* __FLINT_NATIVE_IO_FILE_DESCRIPTOR_H */
