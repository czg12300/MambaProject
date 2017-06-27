//
// Created by jakechen on 2017/6/1.
//

#include "FFmpegEffectJni.h"

JNI(void, nativeHandleEffect)(JNIEnv *env, jobject obj,jboolean  isDealAudio, jint effectType, jstring srcFile_,
                              jstring outFile_, jlong rangStart, jlong rangeEnd, jlong effectStart,
                              jlong effectEnd, jobject listener_) {
    const char *src = env->GetStringUTFChars(srcFile_, 0);
    const char *out = env->GetStringUTFChars(outFile_, 0);
    EffectListener *listener = new EffectListener(env, listener_);
    FFmpegEffect *effect = new FFmpegEffect();
    effect->handleEffect(isDealAudio,effectType, src, out, rangStart, rangeEnd, effectStart, effectEnd,
                         (OnProgressListener *) listener);
    delete (effect);
    delete (listener);
    env->ReleaseStringUTFChars(srcFile_, src);
    env->ReleaseStringUTFChars(outFile_, out);
}

EffectListener::EffectListener(JNIEnv *env, jobject listener_) {
    this->env = env;
    this->listener_ = listener_;
    classId = env->FindClass("com/kugou/showguys/video/effect/OnEffectProgressListener");
    mProgress = env->GetMethodID(classId, "onProgress", "(II)V");
    mSuccess = env->GetMethodID(classId, "onSuccess", "()V");
    mFail = env->GetMethodID(classId, "onFail", "()V");
}

void EffectListener::onProgress(int total, int progress) {
    env->CallVoidMethod(listener_, mProgress, total, progress);
}

void EffectListener::onSuccess() {
    env->CallVoidMethod(listener_, mSuccess);
}

void EffectListener::onFail() {
    env->CallVoidMethod(listener_, mFail);
}

EffectListener::~EffectListener() {
    env->DeleteLocalRef(classId);
    env->DeleteLocalRef(listener_);
}