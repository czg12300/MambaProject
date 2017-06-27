//
// Created by yongfali on 2016/4/7.
//

#ifndef E_LOG_H
#define E_LOG_H

//#define NEW_FILTER
#include <stdlib.h>
#include <android/log.h>

namespace e 
{
#define E_LOG_ALL 		0
#define E_LOG_DEFAULT 	1
#define E_LOG_VERBOSE	2
#define E_LOG_DEBUG		3
#define E_LOG_INFO		4
#define E_LOG_WARN		5
#define E_LOG_ERROR		6
#define E_LOG_FATAL		7
#define E_LOG_SILENT	8


#ifdef __apple__
#	define LOGV(...)
#	define LOGI(...)
#	define LOGD(...)
#	define LOGW(...)
#	define LOGE(...)
#else
#	define LOG_TAG	"KuGouVideo"
#	define CURRENT_LOG_LEVEL E_LOG_DEBUG

#	if (CURRENT_LOG_LEVEL <= E_LOG_VERBOSE)
#		define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#	else
#		define LOGV(...)
#	endif

#	if (CURRENT_LOG_LEVEL <= E_LOG_DEBUG)
#		define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#	else
#		define LOGD(...)
#	endif

#	if (CURRENT_LOG_LEVEL <= E_LOG_INFO)
#		define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#	else
#		define LOGI(...)
#	endif

#	if (CURRENT_LOG_LEVEL <= E_LOG_WARN)
#		define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#	else
#		define LOGW(...)
#	endif
#	endif

#	if (CURRENT_LOG_LEVEL <= E_LOG_ERROR)
#		define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#	else
#		define LOGE(...)
#	endif

#ifndef _T
#	define _T
#endif

}//end namespace
#endif //_E_LOG_H_
