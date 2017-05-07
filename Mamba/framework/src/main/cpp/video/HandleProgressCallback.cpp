//
// Created by jakechen on 2017/3/31.
//

#include "HandleProgressCallback.h"

HandleProgressCallback::HandleProgressCallback(JNIEnv *env, jobject obj, jmethodID method) {
    this->env = env;
    this->obj = obj;
    this->methodOnHandle = method;
}

void HandleProgressCallback::onHandleProgress(int progress, int total) {
    env->CallVoidMethod(obj, methodOnHandle, progress, total);
}