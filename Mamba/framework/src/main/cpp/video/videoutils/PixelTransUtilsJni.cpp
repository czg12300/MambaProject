//
// Created by jakechen on 2017/6/30.
//

#include "PixelTransUtilsJni.h"

JNI(void, rgbaToYuv)(JNIEnv *env, jclass type, jbyteArray src_, jint width, jint height,
                     jbyteArray dest_) {
    jbyte *src = env->GetByteArrayElements(src_, NULL);
    jbyte *dest = env->GetByteArrayElements(dest_, NULL);
    rgbaToYuv((unsigned char *) src, width, height, (unsigned char *) dest);
    env->ReleaseByteArrayElements(src_, src, 0);
    env->ReleaseByteArrayElements(dest_, dest, 0);
}