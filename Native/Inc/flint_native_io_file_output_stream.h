
#ifndef __FLINT_NATIVE_IO_FILE_OUTPUT_STREAM_H
#define __FLINT_NATIVE_IO_FILE_OUTPUT_STREAM_H

#include "flint_native.h"

jvoid NativeFileOutputStream_Open(FNIEnv *env, jobject obj, jstring name, jbool append);
jvoid NativeFileOutputStream_Write(FNIEnv *env, jobject obj, jint b);
jvoid NativeFileOutputStream_WriteBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);
jvoid NativeFileOutputStream_Close(FNIEnv *env, jobject obj);

static constexpr NativeMethod fileOutputStreamMethods[] = {
    NATIVE_METHOD("open",       "(Ljava/lang/String;Z)V", NativeFileOutputStream_Open),
    NATIVE_METHOD("write",      "(I)V",                   NativeFileOutputStream_Write),
    NATIVE_METHOD("writeBytes", "([BII)V",                NativeFileOutputStream_WriteBytes),
    NATIVE_METHOD("close",      "()V",                    NativeFileOutputStream_Close)
};

#endif /* __FLINT_NATIVE_IO_FILE_OUTPUT_STREAM_H */
