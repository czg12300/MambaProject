//
// Created by jakechen on 2017/5/8.
//

#include "FfmpegVideoEncoderJni.h"


//static VideoEncoder *mVideoEncoder;

JNI(jint, nativeStart)(JNIEnv *env, jobject obj, jstring file_, jint codecType,
                       jint keyIFrameInterval,
                       jint width,
                       jint height, jint frameRate, jint bitRate) {
//    LOGD("encode nativeStart");
//    if (mVideoEncoder != NULL && !mVideoEncoder->isStop) {
//        mVideoEncoder->stop();
////        delete (mVideoEncoder);
////        mVideoEncoder = NULL;
//    }
//    LOGD("encode nativeStart mVideoEncoder == NULL &d", mVideoEncoder == NULL);
//    if (mVideoEncoder == NULL) {
//        mVideoEncoder = new VideoEncoder();
//    }
    const char *file = env->GetStringUTFChars(file_, 0);
    int ret = startEncode(file, codecType, keyIFrameInterval, width, height, frameRate,
                          bitRate);
    env->ReleaseStringUTFChars(file_, file);
    LOGD("encode nativeStart");
    return ret;
}


JNI(void, nativeStop)(JNIEnv *env, jobject obj) {
    LOGD("encode nativeStop");
    stopEncode();
//    if (mVideoEncoder != NULL) {
//        mVideoEncoder->stop();
//    }
//    LOGD("encode nativeStop");
//    delete (mVideoEncoder);
//    mVideoEncoder = NULL;
    LOGD("encode nativeStop");
}

JNI(void, nativeEncode)(JNIEnv *env, jobject obj, jbyteArray data, jint width, jint height) {
    LOGD("encode nativeEncode");
    jbyte *jData = env->GetByteArrayElements(data, 0);
    encode((uint8_t *) jData, width, height);
    env->ReleaseByteArrayElements(data, jData, 0);

//    if (mVideoEncoder != NULL) {
//        jbyte *jData = env->GetByteArrayElements(data, 0);
//        mVideoEncoder->encode((uint8_t *) jData, width, height);
//        env->ReleaseByteArrayElements(data, jData, 0);
//    }
}
