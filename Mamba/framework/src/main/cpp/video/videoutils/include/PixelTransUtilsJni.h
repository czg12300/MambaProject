//
// Created by jakechen on 2017/6/30.
//

#ifndef MAMBA_PIXELTRANSUTILSJNI_H
#define MAMBA_PIXELTRANSUTILSJNI_H

#include "jni.h"
#include "FFmpegVideoMerge.h"
#include "FfmpegVideoCut.h"
#include "PixelTransUtils.h"

using namespace video;
extern "C"
{

#define JNI(rettype, name) JNIEXPORT rettype JNICALL Java_com_framework_ndk_videoutils_PixelTransUtils_##name
JNI(void, rgbaToYuv)(JNIEnv *env, jclass type, jbyteArray src_, jint width, jint height,
                     jbyteArray dest_);
JNI(void, yuv420spToyuv420p)(JNIEnv *env, jclass type, jbyteArray src_, jint src_width,
                             jint src_height,
                             jbyteArray dest_, jint dest_width, jint dest_height);
JNI(void, nv21ToYv12)(JNIEnv *env, jclass type,
                      jbyteArray src_, jint width,
                      jint height, jbyteArray dest_,
                      jint destWidth, jint destHeight,
                      jint rotate);
}

#endif
