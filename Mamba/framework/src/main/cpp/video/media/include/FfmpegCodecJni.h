//
// Created by jakechen on 2017/7/24.
//

#ifndef MAMBA_FFMPEGCODECJNI_H
#define MAMBA_FFMPEGCODECJNI_H

#include <jni.h>
#include "FfmpegCodec.h"

using namespace video;
extern "C"
{
#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_media_FfmpegCodec_##name
JNI(void, nativeSetup)(JNIEnv *env, jobject obj);
JNI(void, start)(JNIEnv *env, jobject obj);
JNI(void, nativeConfigure)(JNIEnv *env, jobject obj, jstring key_, jint value);
JNI(jbyteArray, codec)(JNIEnv *env, jobject obj, jbyteArray src_,jint srcSize);
JNI(void, release)(JNIEnv *env, jobject obj);

}

#endif //MAMBA_FFMPEGCODECJNI_H
