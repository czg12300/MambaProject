//
// Created by jakechen on 2017/5/8.
//
#include "jni.h"
//#include "FfmpegVideoEncoder.h"
#include "FfmpegVideoEncoder1.h"

using namespace video;
extern "C"
{

struct fields_t {
    jclass clazz;
    jfieldID context;
};
#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_videoutils_FfmpegEncoder_##name
//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);
JNI(jint, nativeStart)(JNIEnv *env, jobject obj, jstring file_, jint codecType,
                       jint keyIFrameInterval,
                       jint width, jint height, jint frameRate, jint bitRate);
JNI(void, nativeStop)(JNIEnv *env, jobject obj);
JNI(void, nativeEncode)(JNIEnv *env, jobject obj, jbyteArray data, jint width, jint height);
}

