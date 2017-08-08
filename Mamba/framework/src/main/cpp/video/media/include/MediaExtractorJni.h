//
// Created by jakechen on 2017/7/20.
//

#ifndef MAMBA_MEDIAEXTRACTORJNI_H
#define MAMBA_MEDIAEXTRACTORJNI_H

#include "jni.h"
#include "MediaExtractor.h"

using namespace video;
extern "C"
{
#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_media_FfmpegMediaExtractor_##name

//int setDataSource(const char *file);
//
//void seekTo(int timeUs, int mode);
//
//int getTrackCount();
//
//map<char *, void *> *getTrackFormat(int index);
//
//int getTimestamp();
//
//int readSampleData(unsigned char *data);
//
//void selectTrack(int index);
//
//int getTrackIndex();
//
//int getDuration();
//
//void release();

JNI(void, nativeSetup)(JNIEnv *env, jobject obj);

JNI(void, setDataSource)(JNIEnv *env, jobject obj, jstring file_);

JNI(void, seekTo)(JNIEnv *env, jobject obj, jint timeUs, jint mode);

JNI(jint, getTrackCount)(JNIEnv *env, jobject obj);

JNI(jobject, nativeGetTrackFormat)(JNIEnv *env, jobject obj, jint index);

JNI(jint, getTimestamp)(JNIEnv *env, jobject obj);

JNI(jbyteArray, readSampleData)(JNIEnv *env, jobject obj);
JNI(jint, readSampleData1)(JNIEnv *env, jobject obj, jbyteArray data);
JNI(jint, getSampleDataSize)(JNIEnv *env, jobject obj);

JNI(void, selectTrack)(JNIEnv *env, jobject obj, jint index);

JNI(jint, getTrackIndex)(JNIEnv *env, jobject obj);

JNI(jint, getDuration)(JNIEnv *env, jobject obj);

JNI(void, release)(JNIEnv *env, jobject obj);

}
#endif //MAMBA_MEDIAEXTRACTORJNI_H
