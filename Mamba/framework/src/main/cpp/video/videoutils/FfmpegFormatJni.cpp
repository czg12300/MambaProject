//
// Created by jakechen on 2017/5/10.
//

#include "FfmpegFormatJni.h"

JNI(int, muxer)(JNIEnv *env, jobject instance, jstring videoStream_, jstring audioStream_,
                jstring outFile_, jstring rotate_) {
    int ret = -1;
    if (videoStream_ != NULL && audioStream_ != NULL && outFile_ != NULL) {
        const char *videoStream = env->GetStringUTFChars(videoStream_, 0);
        const char *audioStream = env->GetStringUTFChars(audioStream_, 0);
        const char *outFile = env->GetStringUTFChars(outFile_, 0);
        const char *rotate = rotate_ != NULL ? env->GetStringUTFChars(rotate_, 0) : NULL;

        ret = muxing(videoStream, audioStream, outFile, rotate);
        //FfmpegMuxering *A = new FfmpegMuxering();
        //ret = A->MuxerH264Aac(videoStream, audioStream, outFile);
        //delete A;

        env->ReleaseStringUTFChars(videoStream_, videoStream);
        env->ReleaseStringUTFChars(audioStream_, audioStream);
        env->ReleaseStringUTFChars(outFile_, outFile);
        if (rotate_ != NULL) {
            env->ReleaseStringUTFChars(rotate_, rotate);
        }
    }

    return ret;
}

JNI(int, muxerVideoStream)(JNIEnv *env, jobject instance, jstring videoStream_, jstring outFile_,
                           jstring rotate_) {
    int ret = -1;
    if (videoStream_ != NULL && outFile_ != NULL) {
        const char *videoStream = env->GetStringUTFChars(videoStream_, 0);
        const char *outFile = env->GetStringUTFChars(outFile_, 0);
        const char *rotate = rotate_ != NULL ? env->GetStringUTFChars(rotate_, 0) : NULL;
        ret = h264ToFormat(videoStream, outFile, rotate);
        env->ReleaseStringUTFChars(videoStream_, videoStream);
        env->ReleaseStringUTFChars(outFile_, outFile);
        if (rotate_ != NULL) {
            env->ReleaseStringUTFChars(rotate_, rotate);
        }
    }
    return ret;
}


JNI(int, remuxer)(JNIEnv *env, jobject instance, jstring srcFile_, jstring outFile_,
                  jstring rotate_) {
    int ret = -1;
    if (srcFile_ != NULL && outFile_ != NULL) {
        const char *srcFile = env->GetStringUTFChars(srcFile_, 0);
        const char *outFile = env->GetStringUTFChars(outFile_, 0);
        const char *rotate = rotate_ != NULL ? env->GetStringUTFChars(rotate_, 0) : NULL;
        ret = remuxing(srcFile, outFile, rotate);
        env->ReleaseStringUTFChars(srcFile_, srcFile);
        env->ReleaseStringUTFChars(outFile_, outFile);
        if (rotate_ != NULL) {
            env->ReleaseStringUTFChars(rotate_, rotate);
        }
    }
    return ret;
}

JNI(int, demuxer)(JNIEnv *env, jobject instance, jstring srcFile_, jstring videoStream_,
                  jstring audioStream_) {
    int ret = -1;
    if (videoStream_ != NULL && audioStream_ != NULL && srcFile_ != NULL) {
        const char *videoStream = env->GetStringUTFChars(videoStream_, 0);
        const char *audioStream = env->GetStringUTFChars(audioStream_, 0);
        const char *srcFile = env->GetStringUTFChars(srcFile_, 0);
        ret = demuxing(srcFile, videoStream, audioStream);
        env->ReleaseStringUTFChars(videoStream_, videoStream);
        env->ReleaseStringUTFChars(audioStream_, audioStream);
        env->ReleaseStringUTFChars(srcFile_, srcFile);
    }
    return ret;
}

JNI(int, demuxerVideo)(JNIEnv *env, jobject instance, jstring srcFile_, jstring videoStream_) {
    int ret = -1;
    if (videoStream_ != NULL && srcFile_ != NULL) {
        const char *videoStream = env->GetStringUTFChars(videoStream_, 0);
        const char *srcFile = env->GetStringUTFChars(srcFile_, 0);
        ret = demuxingVideo(srcFile, videoStream);
        env->ReleaseStringUTFChars(videoStream_, videoStream);
        env->ReleaseStringUTFChars(srcFile_, srcFile);
    }
    return ret;
}

JNI(int, demuxerAudio)(JNIEnv *env, jobject instance, jstring srcFile_, jstring audioStream_) {
    int ret = -1;
    if (audioStream_ != NULL && srcFile_ != NULL) {
        const char *audioStream = env->GetStringUTFChars(audioStream_, 0);
        const char *srcFile = env->GetStringUTFChars(srcFile_, 0);
        ret = demuxingAudio(srcFile, audioStream);
        env->ReleaseStringUTFChars(audioStream_, audioStream);
        env->ReleaseStringUTFChars(srcFile_, srcFile);
    }
    return ret;
}