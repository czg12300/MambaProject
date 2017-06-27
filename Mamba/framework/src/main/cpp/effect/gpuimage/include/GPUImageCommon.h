//
// Created by yongfali on 2016/4/8.
//

#ifndef E_GPUIMAGE_COMMON_H
#define E_GPUIMAGE_COMMON_H

#include <EGL/egl.h>
#include <GLES/gl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <utility>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <pthread.h>

#include "Common.h"
#include "config.h"

namespace e {

#ifndef WidthBytes
#   define WidthBytes(bits) ((((bits)+31) & (~31)) / 8)
#endif

#ifndef KG_RGB
#   define KG_RGB(r,g,b) ((uint)(((byte)(b)|((uint)((byte)(g))<<8))|(((uint)(byte)(r))<<16)))
#endif

#ifndef CheckPointer
#   define CheckPointer(x, ret) {if (!(x)) return (ret);}
#endif

#ifndef IsValidHandle
#   define IsValidHandle(x) ((x) != NULL)
#endif

#define _STR(x) #x

#ifndef SHADER_STRING
#   define SHADER_STRING(x) _STR(x)
#endif   

//生成对象函数
#define DEFINE_CREATE(__TYPE__) \
static __TYPE__* Create(void) \
{ \
    __TYPE__ *pRet = new(std::nothrow) __TYPE__(); \
    if (pRet && pRet->Initialize()) \
    { \
        return pRet; \
    } \
    else \
    { \
        delete pRet; \
        pRet = NULL; \
        LOGE("%s::Create() failed!!", _STR(__TYPE__));\
        return NULL; \
    } \
}

#define CONSTRUCTOR_ACCES public

#ifdef USE_CREATE_FUNCTION
#   define New(__TYPE__) \
    __TYPE__::Create()
#else
#   define New(__TYPE__) \
    new __TYPE__()
#endif

#define SMALL_IMAGE_WIDTH  360
#define SMALL_IMAGE_HEIGHT 640
static Size kSmallImageSize(SMALL_IMAGE_WIDTH, SMALL_IMAGE_HEIGHT);

#define E_UINT32_MAX 0xffffffff
#define E_UINT64_MAX 0xffffffffffffffff

#ifndef MIN
#   define MIN(a, b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#   define MAX(a, b) ((a)<(b)?(b):(a))
#endif

#ifndef ROUND
#   define ROUND(x) (int)((x) + 0.5)
#endif

template<class T> 
void SafeFree(T** obj)
{
    if (*obj)
    {
        free(*obj);
        *obj = 0;
    }
}

template<class T> 
void SafeDelete(T** obj)
{
    if (*obj)
    {
        delete *obj;
        *obj = 0;
    }
}

template<class T> 
void SafeDeleteArray(T** obj)
{
    if (*obj)
    {
        delete[] (*obj);
        (*obj) = 0;
    }
}

template<class T> 
T square(T a)
{
    return a * a;
}

template<class T> 
void swap(T & a, T & b)
{
    T temp = a; a = b; b = temp;
}

template<class T> 
T clamp(T x, T a, T b)
{
    return (x < a) ? a : (x > b? b: x);
}

template<class T> 
void limit(T & t, T a, T b)
{
    t = (t<a)?a:(t>b)?b:t;
}

}//end namespace

#endif //E_GPUIMAGE_COMMON_H
