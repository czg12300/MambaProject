//
// Created by jakechen on 2017/5/12.
//
#include "jni.h"
#include "FFmpegVideoMerge.h"
#include "FfmpegVideoCut.h"

using namespace video;
extern "C"
{

#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_videoutils_FfmpegCutMergeUtils_##name
JNI(int, nativeCutAudio)(JNIEnv *env, jobject instance, jstring srcFile_, jlong startMillis,
                         jlong endMillis, jstring outFile_);
JNI(int, nativeCutVideo)(JNIEnv *env, jobject instance, jstring srcFile_, jlong startMillis,
                         jlong endMillis, jstring outFile_);
JNI(int, nativeMerge)(JNIEnv *env, jobject instance, jobjectArray files_, jstring outFile_);
}
