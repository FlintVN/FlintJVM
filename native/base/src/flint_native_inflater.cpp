
#include "miniz.h"
#include <stdlib.h>
#include <string.h>
#include "flint.h"
#include "flint_system_api.h"
#include "flint_java_object.h"
#include "flint_array_object.h"
#include "flint_native_inflater.h"

typedef class : public JObject {
public:
    jobject getZsRef() { return getFieldByIndex(0)->getObj(); }
    jbyteArray getBuf() { return (jbyteArray)getFieldByIndex(1)->getObj(); }
    jint getOff() { return getFieldByIndex(2)->getInt32(); }
    jint getLen() { return getFieldByIndex(3)->getInt32(); }
    jbool getFinished() { return (jbool)getFieldByIndex(4)->getInt32(); }
    jbool getNeedDict() { return (jbool)getFieldByIndex(5)->getInt32(); }
    jlong getBytesRead() { return getFieldByIndex(6)->getInt64(); }
    jlong getBytesWritten() { return getFieldByIndex(8)->getInt64(); }

    jvoid setZsRef(jobject val) { getFieldByIndex(0)->setObj(val); }
    jvoid setBuf(jbyteArray val) { getFieldByIndex(1)->setObj(val); }
    jvoid setOff(jint val) { getFieldByIndex(2)->setInt32(val); }
    jvoid setLen(jint val) { getFieldByIndex(3)->setInt32(val); }
    jvoid setFinished(jbool val) { getFieldByIndex(4)->setInt32(val); }
    jvoid setNeedDict(jbool val) { getFieldByIndex(5)->setInt32(val); }
    jvoid setBytesRead(jlong val) { getFieldByIndex(6)->setInt64(val); }
    jvoid setBytesWritten(jlong val) { getFieldByIndex(8)->setInt64(val); }
} *JInflater;

static void throwDataFormatException(FNIEnv *env, const char *msg) {
    env->throwNew(env->findClass("java/util/zip/DataFormatException"), msg);
}

static void throwOutOfMemoryError(FNIEnv *env, const char *msg) {
    env->throwNew(env->findClass("java/lang/OutOfMemoryError"), msg);
}

static mz_stream *getMzStream(uint32_t hook) {
    return (mz_stream *)((Hook *)hook)->getHandle();
}

static void NativeInflater_InflaterEnd(mz_stream *strm) {
    mz_inflateEnd(strm);
    FlintAPI::System::free(strm);
}

jint NativeInflater_Init(FNIEnv *env, jbool nowrap) {
    FExec *exec = (FExec *)env;
    mz_stream *strm = (mz_stream *)FlintAPI::System::malloc(sizeof(mz_stream));
    if(strm == NULL) {
        throwOutOfMemoryError(env, "out of memory allocating inflate stream");
        return 0;
    }
    memset(strm, 0, sizeof(mz_stream));

    int windowBits = nowrap ? -MZ_DEFAULT_WINDOW_BITS : MZ_DEFAULT_WINDOW_BITS;
    int ret = mz_inflateInit2(strm, windowBits);
    if(ret != MZ_OK) {
        FlintAPI::System::free(strm);
        throwDataFormatException(env, "mz_inflateInit2 failed");
        return 0;
    }

    Hook *hook = exec->getFlint()->addShutdownHook(exec, strm, (void (*)(void*))NativeInflater_InflaterEnd);
    if(hook == NULL) {
        inflateEnd(strm);
        return 0;
    }

    return (jint)hook;
}

jvoid NativeInflater_SetDictionary(FNIEnv *env, jint addr, jbyteArray b, jint off, jint len) {
    if(addr == 0) {
        throwDataFormatException(env, "Inflater has been closed");
        return;
    }

    // miniz's zlib-compatible wrapper does not implement inflate/deflate
    // preset dictionaries at all (no mz_inflateSetDictionary exists) - its
    // sliding window lives inside the caller's own output buffer, so
    // supporting this properly would mean re-implementing zlib's dictionary
    // priming (seeding adler32 with adler32(dictionary), pre-filling the
    // 32K window, fixing up tinfl_decompressor's internal distance
    // bookkeeping) on top of miniz's private state.
    //
    // Preset dictionaries are essentially never used by real .zip/.jar/gzip
    // data (they're an opt-in Deflater feature, not something ordinary
    // resource/archive streams emit), so unless you know you need one,
    // it's not worth the risk of poking miniz's private internals for it.
    // Surface that honestly instead of silently pretending it worked:
    (void)b;
    (void)off;
    (void)len;
    env->throwNew(env->findClass("java/lang/UnsupportedOperationException"), "preset dictionaries are not supported by this Inflater implementation");
}

jint NativeInflater_InflateBytes(FNIEnv *env, jobject obj, jint addr, jbyteArray b, jint off, jint len) {
    JInflater inf = (JInflater)obj;
    if(addr == 0) {
        throwDataFormatException(env, "Inflater has been closed");
        return 0;
    }
    mz_stream *strm = getMzStream(addr);

    jbyteArray inBuf = inf->getBuf();
    jint inOff = inf->getOff();
    jint inLen = inf->getLen();

    strm->next_in = (inBuf != NULL) ? (const unsigned char *)inBuf->getData() + inOff : NULL;
    strm->avail_in = (unsigned int)inLen;

    strm->next_out = (unsigned char *)b->getData() + off;
    strm->avail_out = (unsigned int)len;

    int ret = mz_inflate(strm, MZ_NO_FLUSH);

    jint consumed = inLen - (jint)strm->avail_in;
    inf->setOff(inOff + consumed);
    inf->setLen(inLen - consumed);

    jint produced = len - (jint)strm->avail_out;

    switch(ret) {
        case MZ_STREAM_END:
            inf->setFinished(true);
            break;
        case MZ_NEED_DICT:
            inf->setNeedDict(true);
            break;
        case MZ_OK:
        case MZ_BUF_ERROR:
            // MZ_BUF_ERROR here just means "no forward progress possible right
            // now" (needs more input or more output space) - not a real error.
            break;
        default:
            throwDataFormatException(env, strm->msg ? strm->msg : "invalid deflate stream");
            return 0;
    }

    return produced;
}

jint NativeInflater_GetAdler(FNIEnv *env, jint addr) {
    return (addr != 0) ? (jint)getMzStream(addr)->adler : 0;
}

jvoid NativeInflater_Reset(FNIEnv *env, jint addr) {
    if(addr == 0) return;
    mz_inflateReset(getMzStream(addr));
}

jvoid NativeInflater_End(FNIEnv *env, jint addr) {
    if(addr != 0) {
        ((FExec *)env)->getFlint()->removeShutdownHook((Hook *)addr);
        NativeInflater_InflaterEnd(getMzStream(addr));
    }
}
