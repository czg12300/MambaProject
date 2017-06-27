//
// Created by liyongfa on 2016/11/18.
//

#ifndef E_KUGOUEFFECTCOMMON_H
#define E_KUGOUEFFECTCOMMON_H
#include <stdint.h>

namespace e
{
#ifdef _MSC_VER
    #	ifdef __cplusplus
#		ifdef KG_STATIC_LIBRARY
#			define KG_API  extern "C"
#		else
#			ifdef KG_SDK_EXPORTS
#				define KG_API extern "C" __declspec(dllexport)
#			else
#				define KG_API extern "C" __declspec(dllimport)
#			endif
#		endif
#	else
#		ifdef KG_STATIC_LIBRARY
#			define KG_API
#		else
#			ifdef KG_SDK_EXPORTS
#				define KG_API __declspec(dllexport)
#			else
#				define KG_API __declspec(dllimport)
#			endif
#		endif
#	endif
#else /* _MSC_VER */
#	ifdef __cplusplus
#		ifdef KG_SDK_EXPORTS
#			define KG_API  extern "C" __attribute__((visibility ("default")))
#		else
#			define KG_API extern "C"
#		endif
#	else
    #		ifdef KG_SDK_EXPORTS
#			define KG_API __attribute__((visibility ("default")))
#		else
#			define KG_API
#		endif
#	endif
#endif

    //美颜参数类型
    typedef enum {
        kBeautySmoothFactor,		//数值范围[0.0f ~ 1.0f],默认值0.25f
        kBeautyWhiteFactor,			//数值范围[0.0f ~ 1.0f],默认值0.10f
        kBeautySoftlyFactor,		//数值范围[0.0f ~ 1.0f],默认值0.20f
        kBeautyPinklyFactor,		//数值范围[0.0f ~ 1.0f],默认值0.20f
        kBeautySharpenFactor,		//数值范围[0.0f ~ 1.0f],默认值0.50f
        kBeautyBlurFactor,          //数值范围[0.0f ~ 1.0f],默认值0.75f
    }BeautyParams;

    typedef enum{
        KuGouEffectLevelSimple,     //对于性能低的设备
        KuGouEffectLevelNormal,	    //对于性能高的设备
        KuGouEffectLevelAdvanced    //超高性能设备（预留）
    }KuGouEffectProfile;

    typedef enum {
        KG_NV12,
        KG_NV32,
        KG_YUY2,
        KG_I420,
        KG_RGB24,
        KG_RGBA,
        KG_BGR24,
        KG_BGRA,
    }KG_PIX_FORMAT;

    typedef void(*KuGouEffectCallback)(int code, const char* msg);
}

#endif //E_KUGOUEFFECTCOMMON_H
