//
// Created by yongfali on 2016/4/9.
//

#ifndef E_EFFECT_IMPL_H
#define E_EFFECT_IMPL_H

#include "GPUImage.h"

#define BLUR_TYPE_CHANNEL	0
#define BLUR_TYPE_GAUSSIAN  1
#define BLUR_TYPE_BILATERAL 2
#define BLUR_TYPE_SURFACE   3

#define CURRENT_BLUR_INDEX  3

#define MODULE_FACEAR		0
#define MODULE_BEAUTY		1
#define MODULE_STYLE        1
#define MODULE_TITLE        1
#define MODULE_DISPLAY      1
#define MODULE_OUTPUT	    1

#if MODULE_FACEAR
#   include "IFaceTracker.h"
#endif

namespace e
{
    typedef enum {
        kBeautyEnableNone = 0x00,
        kBeautyEnableFaceAR = 0x01,
        kBeautyEnableBeauty = 0x02,
        kBeautyEnableDisplay = 0x04,
        kBeautyEnableOutput = 0x08,
        kBeautyEnableStyle = 0x10,
        kBeautyEnableTitle = 0x20,
        kBeautyEnableVideo = 0x40,
    }KuGouEffectMode;

    class KuGouEffectImpl: public Object
    {
    public:
        static KuGouEffectImpl* Singleton();
        static void ReleaseInstance();
    private:
        KuGouEffectImpl();
        virtual ~KuGouEffectImpl();
    public:
        bool IsValid() const;
        void SetViewport(int x, int y, int width, int height);
        void SetImageRotation(int angle, int flipX, int flipY);
        void SetDeviceRotation(int angle);
        void SetFaceTracker(void);
		void SetOutputSize(int width, int height);
        void SetAnimateCacheSize(const int size);
        void SetAnimatePath(const char* path);
        void SetFaceAREnable(bool enable);
        void SetBeautyEnable(bool enable);
        void SetDisplayEnable(bool enable);
        void SetOutputEnable(bool enable);
        void SetBeautyTexelSpacing(float value);
        void SetBeautyDistanceNormalization(float value);
		void SetBeautySmoothFactor(float value);
		void SetBeautyWhiteFactor(float value);
		void SetBeautySoftlightFactor(float value);
		void SetBeautySaturateFactor(float value);
        void SetBeautySharpenFactor(float value);
        void SetBeautyBlurFactor(float value);
        void SetBeautyLevel(int level);
        void SetStyleEnable(bool enable);
        void SetStyleType(int index);
        void SetTitleEnable(bool enable);
        void SetTitleImage(void* data, int width, int height, int stride, int format);
        void SetVideoEnable(bool enable);
        void Render(void* data, int width, int height, int format);
        void Render(void* dst, void* src, int width, int height, int format);
    protected:
        void JoinFilters(void);

        GPUImageOutput* JoinFaceARFilters(GPUImageOutput* filterParent);
        void BreakFaceARFilters(GPUImageOutput* filterParent);

        GPUImageOutput* JoinBeautyFilters(GPUImageOutput* filterParent);
        void BreakBeautyFilters(GPUImageOutput* filterParent);

        GPUImageOutput* JoinStyleFilters(GPUImageOutput* filterParent);
        void BreakStyleFilters(GPUImageOutput* filterParent);

        GPUImageOutput* JoinTitleFilters(GPUImageOutput* filterParent);
        void BreakTitleFilters(GPUImageOutput* filterParent);

        void BreakFilters(void);
    protected:
        int _effectState;
        bool _needRejoin;

        GLContext* _glContext;
        //input
        GPUImageVideoFilter*    _filterVideo;
        GPUImageCameraFilter* 	_filterCamera;
		//face ar
#if MODULE_FACEAR
        IFaceTracker* _faceTracker;
        GPUImageFaceARFilter*  	_filterFaceAR;
        GPUImageAnimateFilter*  _filterAnimate;
        GPUImageOutputFilter* 	_filterSmall;
#endif
        //beauty
#if MODULE_BEAUTY
    #if CURRENT_BLUR_INDEX == BLUR_TYPE_BILATERAL
        GPUImageBilateralFilter* _filterBlur;
    #elif CURRENT_BLUR_INDEX == BLUR_TYPE_CHANNEL
		GPUImagePinkBlurFilter2* _filterBlur;
    #elif CURRENT_BLUR_INDEX == BLUR_TYPE_SURFACE
        GPUImagePinkBlurFilter3* _filterBlur;
    #endif
        GPUImageSmoothFilter*    _filterSmooth;
        GPUImageSharpenFilter*   _filterSharpen;
#endif
        //style
#if MODULE_STYLE
        CurveSamples*            _styleSamples;
        GPUImageStyleFilter*     _filterStyle;
#endif
        //title
#if MODULE_TITLE
        GPUImageTitleFilter*     _filterTitle;
#endif 
        //display
#if MODULE_DISPLAY
        GPUImageDisplayFilter*   _filterDisplay;
#endif
        //output
#if MODULE_OUTPUT
		GPUImageWriteFilter*	 _filterWriter;
        ImageWriter*             _imageWriter;
#endif

        static KuGouEffectImpl* _singleton;
    };
}
#endif //_E_HiVideoRender_H_
