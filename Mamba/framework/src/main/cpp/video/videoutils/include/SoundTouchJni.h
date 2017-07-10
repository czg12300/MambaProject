//
// Created by jakechen on 2017/7/7.
//

#ifndef MAMBA_AUDIOPCMUTILS_H
#define MAMBA_AUDIOPCMUTILS_H

#include <jni.h>
#include "libsoundtouch/SoundTouch.h"
using namespace soundtouch;
extern "C"
{

#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_videoutils_SoundTouchUtils_##name
JNI(void, setupAudioParameters)(JNIEnv *env, jclass type, jint sampleRate, jint channels);
JNI(void, getSampleByBytes)(JNIEnv *env, jclass type, jbyteArray dest_, jint len);
JNI(void, putSampleByBytes)(JNIEnv *env, jclass type, jbyteArray src_, jint len);
JNI(void, setTempo)(JNIEnv *env, jclass type, jdouble newTempo);
JNI(void, setTempoChange)(JNIEnv *env, jclass type, jdouble newTempo);
JNI(void, setPitch)(JNIEnv *env, jclass type, jdouble newPitch);
JNI(void, setPitchOctaves)(JNIEnv *env, jclass type, jdouble newPitch);
JNI(void, setPitchSemiTones)(JNIEnv *env, jclass type, jint newPitch);
JNI(void, setRateChange)(JNIEnv *env, jclass type, jdouble newRate);
JNI(void, setRate)(JNIEnv *env, jclass type, jdouble newRate);
JNI(void, init)(JNIEnv *env, jclass type);
JNI(void, release)(JNIEnv *env, jclass type);
}

#endif //MAMBA_AUDIOPCMUTILS_H
