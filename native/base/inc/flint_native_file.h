
#ifndef __FLINT_NATIVE_FILE_H
#define __FLINT_NATIVE_FILE_H

#include "flint_native.h"

jchar NativeFile_GetSeparatorChar0(FNIEnv *env);
jchar NativeFile_GetPathSeparatorChar0(FNIEnv *env);
jbool NativeFile_IsAbsolute(FNIEnv *env, jobject file);
jstring NativeFile_GetAbsolutePath(FNIEnv *env, jobject file);
jbool NativeFile_Exists(FNIEnv *env, jobject file);
jbool NativeFile_CanWrite(FNIEnv *env, jobject file);
jbool NativeFile_CanRead(FNIEnv *env, jobject file);
jbool NativeFile_IsFile(FNIEnv *env, jobject file);
jbool NativeFile_IsDirectory(FNIEnv *env, jobject file);
jbool NativeFile_IsHidden(FNIEnv *env, jobject file);
jlong NativeFile_LastModified(FNIEnv *env, jobject file);
jlong NativeFile_Length(FNIEnv *env, jobject file);
jbool NativeFile_Mkdir(FNIEnv *env, jobject file);
jbool NativeFile_RenameTo(FNIEnv *env, jobject file, jobject dest);
jbool NativeFile_Delete0(FNIEnv *env, jobject file);
jbool NativeFile_Rmdir0(FNIEnv *env, jobject file);
jobjectArray NativeFile_List(FNIEnv *env, jobject file);
jstring NativeFile_GetCanonicalPath(FNIEnv *env, jobject file, jstring path);
jbool NativeFile_CreateNewFile(FNIEnv *env, jobject file);

inline constexpr NativeMethod fileMethods[] = {
    NATIVE_METHOD("getSeparatorChar0",     "()C",                   NativeFile_GetSeparatorChar0),
    NATIVE_METHOD("getPathSeparatorChar0", "()C",                   NativeFile_GetPathSeparatorChar0),
    NATIVE_METHOD("isAbsolute",            "()Z",                   NativeFile_IsAbsolute),
    NATIVE_METHOD("getAbsolutePath",       "()Ljava/lang/String;",  NativeFile_GetAbsolutePath),
    NATIVE_METHOD("exists",                "()Z",                   NativeFile_Exists),
    NATIVE_METHOD("canWrite",              "()Z",                   NativeFile_CanWrite),
    NATIVE_METHOD("canRead",               "()Z",                   NativeFile_CanRead),
    NATIVE_METHOD("isFile",                "()Z",                   NativeFile_IsFile),
    NATIVE_METHOD("isDirectory",           "()Z",                   NativeFile_IsDirectory),
    NATIVE_METHOD("isHidden",              "()Z",                   NativeFile_IsHidden),
    NATIVE_METHOD("lastModified",          "()J",                   NativeFile_LastModified),
    NATIVE_METHOD("length",                "()J",                   NativeFile_Length),
    NATIVE_METHOD("mkdir",                 "()Z",                   NativeFile_Mkdir),
    NATIVE_METHOD("renameTo",              "(Ljava/io/File;)Z",     NativeFile_RenameTo),
    NATIVE_METHOD("delete0",               "()Z",                   NativeFile_Delete0),
    NATIVE_METHOD("rmdir0",                "()Z",                   NativeFile_Rmdir0),
    NATIVE_METHOD("list",                  "()[Ljava/lang/String;", NativeFile_List),
    NATIVE_METHOD("getCanonicalPath",      "()Ljava/lang/String;",  NativeFile_GetCanonicalPath),
    NATIVE_METHOD("createNewFile",         "()Z",                   NativeFile_CreateNewFile),
};

#endif /* __FLINT_NATIVE_FILE_H */
