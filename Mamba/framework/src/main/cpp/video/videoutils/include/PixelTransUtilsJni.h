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
}

#endif
