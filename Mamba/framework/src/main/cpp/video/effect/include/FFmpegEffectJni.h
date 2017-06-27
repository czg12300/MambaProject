//
// Created by jakechen on 2017/6/1.
//
#include "jni.h"
#include "OnProgressListener.h"
#include "FFmpegEffect.h"

using namespace video;
extern "C"
{
class EffectListener : public OnProgressListener {
private:
    jclass classId;
    jmethodID mProgress;
    jmethodID mSuccess;
    jmethodID mFail;
    JNIEnv *env;
    jobject listener_;
public:
    EffectListener(JNIEnv *env, jobject listener_);

    ~EffectListener();

    void onProgress(int total, int progress);

    void onSuccess();

    void onFail();
};
#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_effect_FfmpegEffect_##name

JNI(void, nativeHandleEffect)(JNIEnv *env, jobject obj,jboolean  isDealAudio,jint effectType, jstring srcFile_,
                              jstring outFile_, jlong rangStart, jlong rangeEnd, jlong effectStart,
                              jlong effectEnd, jobject listener_);

}
