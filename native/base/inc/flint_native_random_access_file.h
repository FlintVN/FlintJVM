
#ifndef __FLINT_NATIVE_RANDOM_ACCESS_FILE_H
#define __FLINT_NATIVE_RANDOM_ACCESS_FILE_H

#include "flint_native.h"

jvoid NativeRandomAccessFile_Open(FNIEnv *env, jobject obj, jstring name, jint mode);
jint NativeRandomAccessFile_Read(FNIEnv *env, jobject obj);
jint NativeRandomAccessFile_ReadBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);
jvoid NativeRandomAccessFile_Write(FNIEnv *env, jobject obj, jint b);
jvoid NativeRandomAccessFile_WriteBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len);
jlong NativeRandomAccessFile_GetFilePointer(FNIEnv *env, jobject obj);
jvoid NativeRandomAccessFile_Seek(FNIEnv *env, jobject obj, jlong pos);
jlong NativeRandomAccessFile_Length(FNIEnv *env, jobject obj);
jvoid NativeRandomAccessFile_SetLength(FNIEnv *env, jobject obj, jlong newLength);
jvoid NativeRandomAccessFile_Close(FNIEnv *env, jobject obj);

inline constexpr NativeMethod randomAccessFileMethods[] = {
    NATIVE_METHOD("open",              "(Ljava/lang/String;I)V", NativeRandomAccessFile_Open),
    NATIVE_METHOD("read",              "()I",                    NativeRandomAccessFile_Read),
    NATIVE_METHOD("readBytes",         "([BII)I",                NativeRandomAccessFile_ReadBytes),
    NATIVE_METHOD("write",             "(I)V",                   NativeRandomAccessFile_Write),
    NATIVE_METHOD("writeBytes",        "([BII)V",                NativeRandomAccessFile_WriteBytes),
    NATIVE_METHOD("getFilePointer",    "()J",                    NativeRandomAccessFile_GetFilePointer),
    NATIVE_METHOD("seek",              "(J)V",                   NativeRandomAccessFile_Seek),
    NATIVE_METHOD("length",            "()J",                    NativeRandomAccessFile_Length),
    NATIVE_METHOD("setLength",         "(J)V",                   NativeRandomAccessFile_SetLength),
    NATIVE_METHOD("close",             "()V",                    NativeRandomAccessFile_Close),
};

#endif /* __FLINT_NATIVE_RANDOM_ACCESS_FILE_H */
