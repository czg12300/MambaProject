//
//视频格式处理器
// Created by jakechen on 2017/5/10.
//
#include "jni.h"
#include "FfmpegMuxer.h"
#include "FfmpegDemuxer.h"
#include "FfmpegMuxering.h"

using namespace video;
extern "C"
{

#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_videoutils_FfmpegFormatUtils_##name
JNI(int, muxer)(JNIEnv *env, jobject instance, jstring videoStream_, jstring audioStream_,
                jstring outFile_, jstring rotate_);
JNI(int, muxerVideoStream)(JNIEnv *env, jobject instance, jstring videoStream_, jstring outFile_,
                jstring rotate_);
JNI(int, remuxer)(JNIEnv *env, jobject instance, jstring srcFile_, jstring outFile_,
                  jstring rotate_);
JNI(int, demuxer)(JNIEnv *env, jobject instance, jstring srcFile_, jstring videoStream_,
                  jstring audioStream_);
JNI(int, demuxerVideo)(JNIEnv *env, jobject instance, jstring srcFile_, jstring videoStream_);
JNI(int, demuxerAudio)(JNIEnv *env, jobject instance, jstring srcFile_, jstring audioStream_);
}