
#ifndef __FLINT_NATIVE_IO_FILE_H
#define __FLINT_NATIVE_IO_FILE_H

#include "flint_native.h"

jchar NativeIoFile_GetSeparatorChar0(FNIEnv *env);
jchar NativeIoFile_GetPathSeparatorChar0(FNIEnv *env);
jbool NativeIoFile_IsAbsolute(FNIEnv *env, jobject file);
jstring NativeIoFile_GetAbsolutePath(FNIEnv *env, jobject file);
jbool NativeIoFile_Exists(FNIEnv *env, jobject file);
jbool NativeIoFile_CanWrite(FNIEnv *env, jobject file);
jbool NativeIoFile_CanRead(FNIEnv *env, jobject file);
jbool NativeIoFile_IsFile(FNIEnv *env, jobject file);
jbool NativeIoFile_IsDirectory(FNIEnv *env, jobject file);
jbool NativeIoFile_IsHidden(FNIEnv *env, jobject file);
jlong NativeIoFile_LastModified(FNIEnv *env, jobject file);
jlong NativeIoFile_Length(FNIEnv *env, jobject file);
jbool NativeIoFile_Mkdir(FNIEnv *env, jobject file);
jbool NativeIoFile_RenameTo(FNIEnv *env, jobject file, jobject dest);
jbool NativeIoFile_Delete0(FNIEnv *env, jobject file);
jbool NativeIoFile_Rmdir0(FNIEnv *env, jobject file);
jobjectArray NativeIoFile_List(FNIEnv *env, jobject file);
jstring NativeIoFile_GetCanonicalPath(FNIEnv *env, jobject file, jstring path);
jbool NativeIoFile_CreateNewFile(FNIEnv *env, jobject file);

static constexpr NativeMethod ioFileMethods[] = {
    NATIVE_METHOD("getSeparatorChar0",     "()C",                   NativeIoFile_GetSeparatorChar0),
    NATIVE_METHOD("getPathSeparatorChar0", "()C",                   NativeIoFile_GetPathSeparatorChar0),
    NATIVE_METHOD("isAbsolute",            "()Z",                   NativeIoFile_IsAbsolute),
    NATIVE_METHOD("getAbsolutePath",       "()Ljava/lang/String;",  NativeIoFile_GetAbsolutePath),
    NATIVE_METHOD("exists",                "()Z",                   NativeIoFile_Exists),
    NATIVE_METHOD("canWrite",              "()Z",                   NativeIoFile_CanWrite),
    NATIVE_METHOD("canRead",               "()Z",                   NativeIoFile_CanRead),
    NATIVE_METHOD("isFile",                "()Z",                   NativeIoFile_IsFile),
    NATIVE_METHOD("isDirectory",           "()Z",                   NativeIoFile_IsDirectory),
    NATIVE_METHOD("isHidden",              "()Z",                   NativeIoFile_IsHidden),
    NATIVE_METHOD("lastModified",          "()J",                   NativeIoFile_LastModified),
    NATIVE_METHOD("length",                "()J",                   NativeIoFile_Length),
    NATIVE_METHOD("mkdir",                 "()Z",                   NativeIoFile_Mkdir),
    NATIVE_METHOD("renameTo",              "(Ljava/io/File;)Z",     NativeIoFile_RenameTo),
    NATIVE_METHOD("delete0",               "()Z",                   NativeIoFile_Delete0),
    NATIVE_METHOD("rmdir0",                "()Z",                   NativeIoFile_Rmdir0),
    NATIVE_METHOD("list",                  "()[Ljava/lang/String;", NativeIoFile_List),
    NATIVE_METHOD("getCanonicalPath",      "()Ljava/lang/String;",  NativeIoFile_GetCanonicalPath),
    NATIVE_METHOD("createNewFile",         "()Z",                   NativeIoFile_CreateNewFile),
};

#endif /* __FLINT_NATIVE_IO_FILE_H */
