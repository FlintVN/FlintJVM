
#include "Flint.h"
#include "flint_common.h"
#include "flint_native_io_file_input_stream.h"

static jint getFd(FNIEnv *env, jobject obj) {
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd == -1)
        env->throwNew(env->findClass("java/io/IOException"), "File has not been opened");
    return fd;
}

jvoid nativeIoFileInputStreamOpen(FNIEnv *env, jobject obj, jstring name) {
    char buff[FILE_NAME_BUFF_SIZE];
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(resolvePath(name->getAscii(), name->getLength(), buff, sizeof(buff)) == 0) return;
    Flint::lock();
    if(fd != -1)
        env->throwNew(env->findClass("java/io/IOException"), "File has been opened");
    else {
        auto handle = FlintAPI::IO::fopen(buff, FlintAPI::IO::FILE_MODE_READ);
        if(handle == NULL)
            env->throwNew(env->findClass("java/io/FileNotFoundException"), "File opening failed");
        else
            fdObj->getFieldByIndex(0)->setInt32((int32_t)handle);
    }
    Flint::unlock();
}

jint nativeIoFileInputStreamRead(FNIEnv *env, jobject obj) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    else if(fd == 0) {  /* in */
        return -1;
    }
    else if(fd == 1) {  /* out */
        return -1;
    }
    else if(fd == 2) {  /* err */
        return -1;
    }
    else {
        uint8_t data;
        uint32_t br = 0;
        auto result = FlintAPI::IO::fread((FlintAPI::IO::FileHandle)fd, &data, 1, &br);
        if(result != FlintAPI::IO::FILE_RESULT_OK)
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing file");
        return (br == 1) ? data : -1;
    }
}

jint nativeIoFileInputStreamReadBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    if(b == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return 0;
    }
    if(off < 0 || (off + len) > b->getLength()) {
        jclass excpCls = env->findClass("java/lang/IndexOutOfBoundsException");
        if(off < 0)
            env->throwNew(excpCls, "Index %d out of bounds for byte[%d]", off, b->getLength());
        else
            env->throwNew(excpCls, "Last index %d out of bounds for byte[%d]", off + len - 1, b->getLength());
        return 0;
    }
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    else if(fd == 0) {  /* in */
        return 0;
    }
    else if(fd == 1) {  /* out */
        return 0;
    }
    else if(fd == 2) {  /* err */
        return 0;
    }
    else {
        int8_t *buff = &b->getData()[off];
        uint32_t br = 0;
        auto result = FlintAPI::IO::fread((FlintAPI::IO::FileHandle)fd, buff, len, &br);
        if(result != FlintAPI::IO::FILE_RESULT_OK)
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing file");
        return br;
    }
}

jlong nativeIoFileInputStreamLength(FNIEnv *env, jobject obj) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    return FlintAPI::IO::fsize((FlintAPI::IO::FileHandle)fd);
}

jlong nativeIoFileInputStreamPosition(FNIEnv *env, jobject obj) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    return FlintAPI::IO::ftell((FlintAPI::IO::FileHandle)fd);
}

jlong nativeIoFileInputStreamSkip(FNIEnv *env, jobject obj, jlong n) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    if(n > 0xFFFFFFFF) n = 0xFFFFFFFF;
    jint oldPos = FlintAPI::IO::ftell((FlintAPI::IO::FileHandle)fd);
    if(FlintAPI::IO::fseek((FlintAPI::IO::FileHandle)fd, (uint32_t)n) != FlintAPI::IO::FILE_RESULT_OK)
        return 0;
    return oldPos - FlintAPI::IO::ftell((FlintAPI::IO::FileHandle)fd);
}

jint nativeIoFileInputStreamAvailable(FNIEnv *env, jobject obj) {
    jint fd = getFd(env, obj);
    if(fd == -1) return 0;
    auto h = (FlintAPI::IO::FileHandle)fd;
    return FlintAPI::IO::fsize(h) - FlintAPI::IO::ftell(h);
}

jvoid nativeIoFileInputStreamClose(FNIEnv *env, jobject obj) {
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    Flint::lock();
    if(fd != -1) {
        if(!(0 <= fd && fd <= 2)) {
            if(FlintAPI::IO::fclose((FlintAPI::IO::FileHandle)fd) == FlintAPI::IO::FILE_RESULT_OK)
                fdObj->getFieldByIndex(0)->setInt32(-1);
            else
                env->throwNew(env->findClass("java/io/IOException"), "File closing failed");
        }
    }
    else
        env->throwNew(env->findClass("java/io/IOException"), "File has not been opened");
    Flint::unlock();
}
