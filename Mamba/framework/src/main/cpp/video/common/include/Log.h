//
// Created by jakechen on 2017/1/4.
//
#ifndef GITXIUGE_LOG_H_H
#define GITXIUGE_LOG_H_H

#include <android/log.h>

#define TAG "ndk-log"

#define USELOG 1
int test_android_no_log_print_debuge(int prio, const char *tag, const char *fmt, ...);

#if  USELOG
    #define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
    #define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
    #define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
    #define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,TAG,__VA_ARGS__)
    #define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#else
    #define LOGD(...)  test_android_no_log_print_debuge(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
    #define LOGE(...)  test_android_no_log_print_debuge(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)
    #define LOGI(...)  test_android_no_log_print_debuge(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
    #define LOGV(...)  test_android_no_log_print_debuge(ANDROID_LOG_VERBOSE,TAG,__VA_ARGS__)
    #define LOGW(...)  test_android_no_log_print_debuge(ANDROID_LOG_WARN,TAG,__VA_ARGS__)
#endif

#endif