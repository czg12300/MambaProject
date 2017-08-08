//
// Created by jakechen on 2017/7/24.
//

#include "FfmpegCodecJni.h"

static FfmpegCodec *codec = NULL;

JNI(void, nativeSetup)(JNIEnv *env, jobject obj) {
    codec = new FfmpegCodec();
}

JNI(void, nativeConfigure)(JNIEnv *env, jobject obj, jstring key_, jint value) {
    if (codec != NULL) {
        const char *key = env->GetStringUTFChars(key_, 0);
        codec->_configure(key, value);
        env->ReleaseStringUTFChars(key_, key);
    }
}

JNI(void, start)(JNIEnv *env, jobject obj) {
    if (codec != NULL) {
        codec->start();
    }
}

JNI(jbyteArray, codec)(JNIEnv *env, jobject obj, jbyteArray src_, jint srcSize) {
    jbyte *jData = env->GetByteArrayElements(src_, 0);
//data是结构体pImageData中的byte[];
    if (codec != NULL) {
        int len = 0;
        uint8_t *data = codec->_decodeOrEncode((uint8_t *) jData, srcSize, &len);
        jbyte *temp = (jbyte *) data;
        jbyteArray jarrRV = env->NewByteArray(len);
        LOGD("readSampleData jni len %d", len);
        env->SetByteArrayRegion(jarrRV, 0, len, temp);
        return jarrRV;
    }
    env->ReleaseByteArrayElements(src_, jData, 0);
    return NULL;
}

JNI(void, release)(JNIEnv *env, jobject obj) {
    if (codec != NULL) {
        codec->release();
        delete codec;
        codec = NULL;
    }
}