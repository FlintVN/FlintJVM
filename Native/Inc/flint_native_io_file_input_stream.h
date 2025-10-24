
#ifndef __FLINT_NATIVE_IO_FILE_INPUT_STREAM_H
#define __FLINT_NATIVE_IO_FILE_INPUT_STREAM_H

#include "flint_native.h"

jvoid nativeIoFileInputStreamOpen(FNIEnv *env, jobject obj, jstring name);
jint nativeIoFileInputStreamRead(FNIEnv *env, jobject obj);
jint nativeIoFileInputStreamReadBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);
jlong nativeIoFileInputStreamLength(FNIEnv *env, jobject obj);
jlong nativeIoFileInputStreamPosition(FNIEnv *env, jobject obj);
jlong nativeIoFileInputStreamSkip(FNIEnv *env, jobject obj, jlong n);
jint nativeIoFileInputStreamAvailable(FNIEnv *env, jobject obj);
jvoid nativeIoFileInputStreamClose(FNIEnv *env, jobject obj);

static constexpr NativeMethod ioFileInputStreamMethods[] = {
    NATIVE_METHOD("open",      "(Ljava/lang/String;)V", nativeIoFileInputStreamOpen),
    NATIVE_METHOD("read",      "()I",                   nativeIoFileInputStreamRead),
    NATIVE_METHOD("readBytes", "([BII)I",               nativeIoFileInputStreamReadBytes),
    NATIVE_METHOD("length",    "()J",                   nativeIoFileInputStreamLength),
    NATIVE_METHOD("position",  "()J",                   nativeIoFileInputStreamPosition),
    NATIVE_METHOD("skip",      "(J)J",                  nativeIoFileInputStreamSkip),
    NATIVE_METHOD("available", "()I",                   nativeIoFileInputStreamAvailable),
    NATIVE_METHOD("close",     "()V",                   nativeIoFileInputStreamClose),
};

#endif /* __FLINT_NATIVE_IO_FILE_INPUT_STREAM_H */
