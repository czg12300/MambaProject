//
// Created by jakechen on 2017/3/31.
//
#include <jni.h>

#ifndef XIUGE_HANDLEPROGRESSCALLBACK_H
#define XIUGE_HANDLEPROGRESSCALLBACK_H


class HandleProgressCallback {
private:
    JNIEnv *env;
    jobject obj;
    jmethodID methodOnHandle;
public:
    HandleProgressCallback(JNIEnv *env, jobject obj, jmethodID method);

    void onHandleProgress(int progress, int total);

};

class ProgressCallback {
public:
    virtual void onProgress(int progress, int total)=0;

    virtual void onFinish(int progress, int total)=0;
};

#endif //XIUGE_HANDLEPROGRESSCALLBACK_H
