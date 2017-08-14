//
// Created by jakechen on 2017/7/7.
//

#ifndef MAMBA_AUDIOPCMUTILS_H
#define MAMBA_AUDIOPCMUTILS_H

#include <jni.h>
#include "SoundTouchStream.h"

using namespace video;
extern "C"
{

#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_media_SoundTouch_##name
JNI(void, clearBytes)(JNIEnv *env, jclass type, jint track);
JNI(void, setup)(JNIEnv *env, jobject thiz, jint track, jint channels, jint samplingRate,
                 jint bytesPerSample, jfloat tempo, jfloat pitchSemi);
JNI(void, finish)(JNIEnv *env, jobject thiz, jint track, int length);
JNI(void, putBytes)(JNIEnv *env, jobject thiz, jint track, jbyteArray input, jint length);
JNI(jint, getBytes)(JNIEnv *env, jobject thiz, jint track, jbyteArray get, jint toGet);
JNI(void, setPitchSemi)(JNIEnv *env, jobject thiz, jint track, jfloat pitchSemi);
JNI(void, setTempo)(JNIEnv *env, jobject thiz, jint track, jfloat tempo);
JNI(void, setRate)(JNIEnv *env, jobject thiz, jint track, jfloat rate);
JNI(void, setRateChange)(JNIEnv *env, jobject thiz, jint track, jfloat rate);
JNI(jlong, getOutputBufferSize)(JNIEnv *env, jobject thiz, jint track);
JNI(void, setTempoChange)(JNIEnv *env, jobject thiz, jint track, jfloat tempoChange);
JNI(void, setSpeech)(JNIEnv *env, jobject thiz, jint track, jboolean speech);
}
#endif //MAMBA_AUDIOPCMUTILS_H
