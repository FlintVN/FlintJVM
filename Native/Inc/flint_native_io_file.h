
#ifndef __FLINT_NATIVE_IO_FILE_H
#define __FLINT_NATIVE_IO_FILE_H

#include "flint_native.h"

jchar nativeIoFileGetSeparatorChar0(FNIEnv *env);
jchar nativeIoFileGetPathSeparatorChar0(FNIEnv *env);
jbool nativeIoFileIsAbsolute(FNIEnv *env, jobject file);
jstring nativeIoFileGetAbsolutePath(FNIEnv *env, jobject file);
jbool nativeIoFileExists(FNIEnv *env, jobject file);
jbool nativeIoFileCanWrite(FNIEnv *env, jobject file);
jbool nativeIoFileCanRead(FNIEnv *env, jobject file);
jbool nativeIoFileIsFile(FNIEnv *env, jobject file);
jbool nativeIoFileIsDirectory(FNIEnv *env, jobject file);
jbool nativeIoFileIsHidden(FNIEnv *env, jobject file);
jlong nativeIoFileLastModified(FNIEnv *env, jobject file);
jlong nativeIoFileLength(FNIEnv *env, jobject file);
jbool nativeIoFileMkdir(FNIEnv *env, jobject file);
jbool nativeIoFileRenameTo(FNIEnv *env, jobject file, jobject dest);
jbool nativeIoFileDelete0(FNIEnv *env, jobject file);
jbool nativeIoFileRmdir0(FNIEnv *env, jobject file);
jobjectArray nativeIoFileList(FNIEnv *env, jobject file);
jstring nativeIoFileGetCanonicalPath(FNIEnv *env, jobject file, jstring path);
jbool nativeIoFileCreateNewFile(FNIEnv *env, jobject file);

static constexpr NativeMethod ioFileMethods[] = {
    NATIVE_METHOD("getSeparatorChar0",     "()C",                   nativeIoFileGetSeparatorChar0),
    NATIVE_METHOD("getPathSeparatorChar0", "()C",                   nativeIoFileGetPathSeparatorChar0),
    NATIVE_METHOD("isAbsolute",            "()Z",                   nativeIoFileIsAbsolute),
    NATIVE_METHOD("getAbsolutePath",       "()Ljava/lang/String;",  nativeIoFileGetAbsolutePath),
    NATIVE_METHOD("exists",                "()Z",                   nativeIoFileExists),
    NATIVE_METHOD("canWrite",              "()Z",                   nativeIoFileCanWrite),
    NATIVE_METHOD("canRead",               "()Z",                   nativeIoFileCanRead),
    NATIVE_METHOD("isFile",                "()Z",                   nativeIoFileIsFile),
    NATIVE_METHOD("isDirectory",           "()Z",                   nativeIoFileIsDirectory),
    NATIVE_METHOD("isHidden",              "()Z",                   nativeIoFileIsHidden),
    NATIVE_METHOD("lastModified",          "()J",                   nativeIoFileLastModified),
    NATIVE_METHOD("length",                "()J",                   nativeIoFileLength),
    NATIVE_METHOD("mkdir",                 "()Z",                   nativeIoFileMkdir),
    NATIVE_METHOD("renameTo",              "(Ljava/io/File;)Z",     nativeIoFileRenameTo),
    NATIVE_METHOD("delete0",               "()Z",                   nativeIoFileDelete0),
    NATIVE_METHOD("rmdir0",                "()Z",                   nativeIoFileRmdir0),
    NATIVE_METHOD("list",                  "()[Ljava/lang/String;", nativeIoFileList),
    NATIVE_METHOD("getCanonicalPath",      "()Ljava/lang/String;",  nativeIoFileGetCanonicalPath),
    NATIVE_METHOD("createNewFile",         "()Z",                   nativeIoFileCreateNewFile),
};

#endif /* __FLINT_NATIVE_IO_FILE_H */
