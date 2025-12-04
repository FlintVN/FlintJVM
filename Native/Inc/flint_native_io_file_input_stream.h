
#ifndef __FLINT_NATIVE_IO_FILE_INPUT_STREAM_H
#define __FLINT_NATIVE_IO_FILE_INPUT_STREAM_H

#include "flint_native.h"

jvoid NativeFileInputStream_Open(FNIEnv *env, jobject obj, jstring name);
jint NativeFileInputStream_Read(FNIEnv *env, jobject obj);
jint NativeFileInputStream_ReadBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);
jlong NativeFileInputStream_Length(FNIEnv *env, jobject obj);
jlong NativeFileInputStream_Position(FNIEnv *env, jobject obj);
jlong NativeFileInputStream_Skip(FNIEnv *env, jobject obj, jlong n);
jint NativeFileInputStream_Available(FNIEnv *env, jobject obj);
jvoid NativeFileInputStream_Close(FNIEnv *env, jobject obj);

static constexpr NativeMethod fileInputStreamMethods[] = {
    NATIVE_METHOD("open",      "(Ljava/lang/String;)V", NativeFileInputStream_Open),
    NATIVE_METHOD("read",      "()I",                   NativeFileInputStream_Read),
    NATIVE_METHOD("readBytes", "([BII)I",               NativeFileInputStream_ReadBytes),
    NATIVE_METHOD("length",    "()J",                   NativeFileInputStream_Length),
    NATIVE_METHOD("position",  "()J",                   NativeFileInputStream_Position),
    NATIVE_METHOD("skip",      "(J)J",                  NativeFileInputStream_Skip),
    NATIVE_METHOD("available", "()I",                   NativeFileInputStream_Available),
    NATIVE_METHOD("close",     "()V",                   NativeFileInputStream_Close),
};

#endif /* __FLINT_NATIVE_IO_FILE_INPUT_STREAM_H */
