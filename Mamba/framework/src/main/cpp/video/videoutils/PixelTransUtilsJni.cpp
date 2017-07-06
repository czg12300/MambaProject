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

JNI(void, yuv420spToyuv420p)(JNIEnv *env, jclass type, jbyteArray src_, jint src_width,
                             jint src_height,
                             jbyteArray dest_, jint dest_width, jint dest_height) {
    jbyte *src = env->GetByteArrayElements(src_, NULL);
    jbyte *dest = env->GetByteArrayElements(dest_, NULL);
    yuv420spToYuv420p((unsigned char *) src, src_width, src_height, (unsigned char *) dest,
                      dest_width, dest_height);
    env->ReleaseByteArrayElements(src_, src, 0);
    env->ReleaseByteArrayElements(dest_, dest, 0);
}

JNI(void, nv21ToYv12)(JNIEnv *env, jclass type,
                      jbyteArray src_, jint width,
                      jint height, jbyteArray dest_,
                      jint destWidth, jint destHeight,
                      jint rotate) {
    jbyte *src = env->GetByteArrayElements(src_, NULL);
    jbyte *dest = env->GetByteArrayElements(dest_, NULL);
    nv21ToYv12((unsigned char *) src, width, height, (unsigned char *) dest,
               destWidth, destHeight, rotate);
    env->ReleaseByteArrayElements(src_, src, 0);
    env->ReleaseByteArrayElements(dest_, dest, 0);
}

JNI(void, nv21ToI420)(JNIEnv *env, jclass type,
                      jbyteArray src_, jint width,
                      jint height, jbyteArray dest_,
                      jint destWidth, jint destHeight,
                      jint rotate) {
    jbyte *src = env->GetByteArrayElements(src_, NULL);
    jbyte *dest = env->GetByteArrayElements(dest_, NULL);
    nv21ToI420((unsigned char *) src, width, height, (unsigned char *) dest,
               destWidth, destHeight, rotate);
    env->ReleaseByteArrayElements(src_, src, 0);
    env->ReleaseByteArrayElements(dest_, dest, 0);
}