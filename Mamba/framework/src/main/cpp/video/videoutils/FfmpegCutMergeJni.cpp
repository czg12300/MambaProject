//
// Created by jakechen on 2017/5/12.
//


#include "FfmpegCutMergeJni.h"

JNI(int, nativeCutAudio)(JNIEnv *env, jobject instance, jstring srcFile_, jlong startMillis,
                         jlong endMillis, jstring outFile_) {
    const char *srcFile = env->GetStringUTFChars(srcFile_, 0);
    const char *outFile = env->GetStringUTFChars(outFile_, 0);
    int ret = cutAudio(srcFile, startMillis, endMillis, outFile);
    env->ReleaseStringUTFChars(srcFile_, srcFile);
    env->ReleaseStringUTFChars(outFile_, outFile);
    return ret;
}

JNI(int, nativeCutVideo)(JNIEnv *env, jobject instance, jstring srcFile_, jlong startMillis,
                         jlong endMillis, jstring outFile_) {
    const char *srcFile = env->GetStringUTFChars(srcFile_, 0);
    const char *outFile = env->GetStringUTFChars(outFile_, 0);
    int ret = cutVideo(srcFile, startMillis, endMillis, outFile);
    env->ReleaseStringUTFChars(srcFile_, srcFile);
    env->ReleaseStringUTFChars(outFile_, outFile);
    return ret;
}

JNI(int, nativeMerge)(JNIEnv *env, jobject instance, jobjectArray files_, jstring outFile_) {
    const char *outFile = env->GetStringUTFChars(outFile_, 0);
    int stringCount = env->GetArrayLength(files_);
    queue<string> srcFiles;
    const char **srcs = new const char *[stringCount];
    for (int i = 0; i < stringCount; i++) {
        jstring jstr = (jstring) (env->GetObjectArrayElement(files_, i));
        const char *temp = env->GetStringUTFChars(jstr, 0);
        srcs[i] = temp;
        srcFiles.push(temp);
    }
    int ret = merge(srcFiles, outFile);
    for (int i = 0; i < stringCount; i++) {
        jstring jstr = (jstring) (env->GetObjectArrayElement(files_, i));
        env->ReleaseStringUTFChars(jstr, srcs[i]);
    }
    env->ReleaseStringUTFChars(outFile_, outFile);
    return ret;
}
