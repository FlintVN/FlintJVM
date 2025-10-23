
#ifndef __FLINT_NATIVE_IO_FILE_OUTPUT_STREAM_H
#define __FLINT_NATIVE_IO_FILE_OUTPUT_STREAM_H

#include "flint_native.h"

jvoid nativeIoFileOutputStreamOpen(FNIEnv *env, jobject obj, jstring name, jbool append);
jvoid nativeIoFileOutputStreamWrite(FNIEnv *env, jobject obj, jint b);
jvoid nativeIoFileOutputStreamWriteBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);
jvoid nativeIoFileOutputStreamClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod ioFileOutputStreamMethods[] = {
    NATIVE_METHOD("open",       "(Ljava/lang/String;Z)V", nativeIoFileOutputStreamOpen),
    NATIVE_METHOD("write",      "(I)V",                   nativeIoFileOutputStreamWrite),
    NATIVE_METHOD("writeBytes", "([BII)V",                nativeIoFileOutputStreamWriteBytes),
    NATIVE_METHOD("close",      "()V",                    nativeIoFileOutputStreamClose)
};

#endif /* __FLINT_NATIVE_IO_FILE_OUTPUT_STREAM_H */
