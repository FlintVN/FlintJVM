
#include "Flint.h"
#include "flint_common.h"
#include "flint_native_io_file_output_stream.h"

jvoid nativeIoFileOutputStreamOpen(FNIEnv *env, jobject obj, jstring name, jbool append) {
    char buff[FILE_NAME_BUFF_SIZE];
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(resolvePath(name->getAscii(), name->getLength(), buff, sizeof(buff)) == -1) return;
    Flint::lock();
    if(fd != -1)
        env->throwNew(env->findClass("java/io/IOException"), "File has been opened");
    else {
        FlintAPI::IO::FileMode fileMode;
        if(append)
            fileMode = (FlintAPI::IO::FileMode)(FlintAPI::IO::FILE_MODE_APPEND | FlintAPI::IO::FILE_MODE_WRITE);
        else
            fileMode = (FlintAPI::IO::FileMode)(FlintAPI::IO::FILE_MODE_CREATE_ALWAYS | FlintAPI::IO::FILE_MODE_WRITE);
        auto handle = FlintAPI::IO::fopen(buff, fileMode);
        if(handle == NULL)
            env->throwNew(env->findClass("java/io/FileNotFoundException"), "File opening failed");
        else
            fdObj->getFieldByIndex(0)->setInt32((int32_t)handle);
    }
    Flint::unlock();
}

jvoid nativeIoFileOutputStreamWrite(FNIEnv *env, jobject obj, jint b) {
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd == -1)
        env->throwNew(env->findClass("java/io/IOException"), "File has not been opened");
    else if(fd == 0) {  /* in */

    }
    else if(fd == 1) {  /* out */
        uint8_t buff = (uint8_t)b;
        Flint::consoleWrite(&buff, 1);
    }
    else if(fd == 2) {  /* err */

    }
    else {
        uint8_t data = b & 0xFF;
        uint32_t bw = 0;
        auto result = FlintAPI::IO::fwrite((FlintAPI::IO::FileHandle)fd, &data, 1, &bw);
        if(result != FlintAPI::IO::FILE_RESULT_OK || bw != 1)
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing file");
    }
}

jvoid nativeIoFileOutputStreamWriteBytes(FNIEnv *env, jobject obj, jbyteArray b, jint off, jint len) {
    if(b == NULL) {
        env->throwNew(env->findClass("java/lang/NullPointerException"));
        return;
    }
    if(off < 0 || (off + len) > b->getLength()) {
        jclass excpCls = env->findClass("java/lang/IndexOutOfBoundsException");
        if(off < 0)
            env->throwNew(excpCls, "Index %d out of bounds for byte[%d]", off, b->getLength());
        else
            env->throwNew(excpCls, "Last index %d out of bounds for byte[%d]", off + len - 1, b->getLength());
        return;
    }
    jobject fdObj = obj->getFieldByIndex(0)->getObj();
    jint fd = fdObj->getFieldByIndex(0)->getInt32();
    if(fd == -1)
        env->throwNew(env->findClass("java/io/IOException"), "File has not been opened");
    else if(fd == 0) {  /* in */

    }
    else if(fd == 1)    /* out */
        Flint::consoleWrite((uint8_t *)b->getData(), b->getLength());
    else if(fd == 2) {  /* err */

    }
    else {
        int8_t *data = &b->getData()[off];
        uint32_t bw = 0;
        auto result = FlintAPI::IO::fwrite((FlintAPI::IO::FileHandle)fd, data, len, &bw);
        if(result != FlintAPI::IO::FILE_RESULT_OK || bw != len)
            env->throwNew(env->findClass("java/io/IOException"), "Error while writing file");
    }
}

jvoid nativeIoFileOutputStreamClose(FNIEnv *env, jobject obj) {
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
