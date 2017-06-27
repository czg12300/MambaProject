//
// Created by yongfali on 2016/4/9.
//

#include "include/KuGouEffectImpl.h"

#if MODULE_FACEAR
#   include "CVFaceTracker.h"
#endif

#define BIT_SHIFT(n) ((1<<n) & 0x00000000)

namespace e
{
    KuGouEffectImpl* KuGouEffectImpl::_singleton = 0;
    KuGouEffectImpl* KuGouEffectImpl::Singleton()
    {
        if (_singleton == 0){
            _singleton = new KuGouEffectImpl();
            assert(_singleton);
        }
        return _singleton;
    }

    void KuGouEffectImpl::ReleaseInstance()
    {
        if (_singleton){
            delete _singleton;
            _singleton = 0;
        }
    }

    KuGouEffectImpl::KuGouEffectImpl()
    {
        _effectState = kBeautyEnableNone;
        _needRejoin = false;

        //初始化一些全局变量，比如paths
        _glContext = new GLContext();
        assert(_glContext);

        //只有GPUImageCameraFilter才会把上层的rotation传递给下一个target
        //其他filter不会把本身的rotation传递给下一个target
        _filterVideo = New(GPUImageVideoFilter);
        _filterCamera = New(GPUImageCameraFilter);
#if MODULE_DISPLAY
        _filterDisplay = New(GPUImageDisplayFilter);
#endif

#if MODULE_OUTPUT
        _imageWriter = new ImageWriter();
        assert(_imageWriter);
		_filterWriter = New(GPUImageWriteFilter);
		//_filterWriter->SetOutputCallback(GPUImageCallback(_imageWriter, &ImageWriter::OnSampleProc));
#endif
        //特效的滤镜
#if MODULE_FACEAR
		_faceTracker = new CVFaceTracker();
        assert(_faceTracker);
        _filterFaceAR = New(GPUImageFaceARFilter);
        _filterAnimate = New(GPUImageAnimateFilter);
        _filterSmall = New(GPUImageOutputFilter);
#endif

#if MODULE_BEAUTY
        //美颜的滤镜
#   if CURRENT_BLUR_INDEX == BLUR_TYPE_BILATERAL
		_filterBlur = New(GPUImageBilateralFilter);
#   elif CURRENT_BLUR_INDEX == BLUR_TYPE_CHANNEL
		_filterBlur = New(GPUImagePinkBlurFilter2);
#   elif CURRENT_BLUR_INDEX == BLUR_TYPE_SURFACE
        _filterBlur = GPUImagePinkBlurFilter3::Create();
        _filterBlur->Initialize();
#   endif

        _filterSmooth = New(GPUImageSmoothFilter);
        _filterSharpen = New(GPUImageSharpenFilter);
#endif//MODULE_BEAUTY

#if MODULE_STYLE
        _styleSamples = new CurveSamples();
        assert(_styleSamples);
        _filterStyle = New(GPUImageStyleFilter);
#endif

#if MODULE_TITLE    
        _filterTitle = New(GPUImageTitleFilter);
#endif

        //设置各个filter默认值
#if 0        
        SetBeautyBlurFactor(0.75f);
        SetBeautySmoothFactor(0.85f);
        SetBeautyWhiteFactor(0.10f);
        SetBeautySoftlightFactor(0.20f);
        SetBeautySaturateFactor(0.3f);
        SetBeautySharpenFactor(0.20f);
#else
        SetBeautyBlurFactor(0.65f);
        SetBeautySmoothFactor(0.75f);
        SetBeautyWhiteFactor(0.3f);
        SetBeautySoftlightFactor(0.3f);
        SetBeautySaturateFactor(0.4f);
        SetBeautySharpenFactor(0.3f);
#endif
        //设置识别模块
        SetFaceTracker(); 
        //需要激活的模块
        _effectState |= kBeautyEnableBeauty;
        _effectState |= kBeautyEnableStyle;
        _effectState |= kBeautyEnableTitle;
        _effectState |= kBeautyEnableDisplay;
        _effectState |= kBeautyEnableOutput;
		
        //链接功能filter
        JoinFilters();
    }

    KuGouEffectImpl::~KuGouEffectImpl()
    {
        BreakFilters();
        SafeDelete(&_filterCamera);
        SafeDelete(&_filterVideo);
#if MODULE_DISPLAY
        SafeDelete(&_filterDisplay);
#endif

#if MODULE_OUTPUT
        SafeDelete(&_imageWriter);
		SafeDelete(&_filterWriter);
#endif

#if MODULE_FACEAR
        SafeDelete(&_filterFaceAR);
        SafeDelete(&_filterAnimate);
        SafeDelete(&_filterSmall);
	    SafeDelete(&_faceTracker);
#endif

#if MODULE_BEAUTY
        SafeDelete(&_filterBlur);
        SafeDelete(&_filterSmooth);
        SafeDelete(&_filterSharpen);
#endif

#if MODULE_STYLE
        SafeDelete(&_styleSamples);
        SafeDelete(&_filterStyle);
#endif

#if MODULE_TITLE    
        SafeDelete(&_filterTitle);
#endif
        SafeDelete(&_glContext);
    }

    bool KuGouEffectImpl::IsValid(void) const
    {
        bool flag = true;
#if MODULE_BEAUTY
        flag &= (_filterBlur && _filterSmooth && _filterSharpen);
#endif

#if MODULE_FACEAR
        flag &= (_filterFaceAR != 0);
#endif

#if MODULE_STYLE
        flag &= (_filterStyle != 0);
#endif 

#if MODULE_TITLE
        flag &= (_filterTitle != 0);
#endif
        return flag;        
    }

    void KuGouEffectImpl::SetViewport(int x, int y, int width, int height)
    {
#if MODULE_DISPLAY        
        if (_filterDisplay){
            _filterDisplay->SetViewport(x, y, width, height);
        }
#endif        
    }

    void KuGouEffectImpl::SetImageRotation(int angle, int flipX, int flipY)
    {
        if (_effectState & kBeautyEnableVideo)
        {
            if (_filterVideo){
                GPUImageRotationMode rotation(angle, flipX, flipY);
                _filterVideo->SetInputRotationMode(rotation, 0);
            }
        }
        else
        {
            if (_filterCamera){
                GPUImageRotationMode rotation(angle, flipX, flipY);
                _filterCamera->SetInputRotationMode(rotation, 0);
            }
        }
    }

    void KuGouEffectImpl::SetDeviceRotation(int degrees)
    {
#if MODULE_FACEAR        
        if (_faceTracker) {
            //TODO: add code
        }
#endif        
    }

	void KuGouEffectImpl::SetOutputSize(int width, int height)
	{
        if (_filterWriter)
        {
            _filterWriter->SetOutputSize(Size(width, height));
        }

        if (_effectState & kBeautyEnableVideo)
        {
            if (_filterVideo){
                _filterVideo->SetOutputSize(Size(width, height));
            }
        }
        else
        {
            if(_filterCamera) {
                _filterCamera->SetOutputSize(Size(width, height));
            }
        }
	}

    void KuGouEffectImpl::SetAnimateCacheSize(const int size)
    {
#if MODULE_FACEAR
        if (_filterAnimate){
			_filterAnimate->SetCacheSize(size);
        }
#endif
    }

    void KuGouEffectImpl::SetAnimatePath(const char* path)
    {
#if MODULE_FACEAR
        if (_filterAnimate){
			_filterAnimate->SetAnimatePath(path);
        }
#endif
    }

    void KuGouEffectImpl::SetFaceTracker(void)
    {
#if MODULE_FACEAR
        //接受图片的回调
        if (_faceTracker && _filterSmall){
            _filterSmall->SetSampleCallback(GPUImageCallback(_faceTracker, &IFaceTracker::OnSampleProc));  
        }

        //处理后结果回调
        if (_faceTracker && _filterAnimate){
            _faceTracker->SetTrackCallback(GPUImageCallback(_filterAnimate, &GPUImageAnimateFilter::OnTrackProc));
        }
#endif
    }

    void KuGouEffectImpl::SetBeautyTexelSpacing(float value)
    {
#if MODULE_BEAUTY && CURRENT_BLUR_INDEX == BLUR_TYPE_BILATERAL
        if (_filterBlur){
			_filterBlur->SetTexelSpacingMultiplier(value);
		}
#endif
    }
    
    void KuGouEffectImpl::SetBeautyDistanceNormalization(float value)
    {
#if MODULE_BEAUTY & CURRENT_BLUR_INDEX == BLUR_TYPE_BILATERAL
        if (_filterBlur){
			_filterBlur->SetDistanceFactor(value);
		}
#endif
    }

	void KuGouEffectImpl::SetBeautySmoothFactor(float value)
	{
#if MODULE_BEAUTY         
		if (_filterSmooth) 
        {
            _filterSmooth->SetSmoothFactor(value);

            if (value <= 0.0f)
            {
                if (_effectState & kBeautyEnableBeauty)
                {
                    SetBeautyEnable(false);
                }
            }
            else
            {
                if (!(_effectState & kBeautyEnableBeauty))
                {
                    SetBeautyEnable(true);
                }
            }
		}
#endif        
	}

	void KuGouEffectImpl::SetBeautyWhiteFactor(float value)
	{
#if MODULE_BEAUTY         
		// if (_filterSmooth) {
		// 	value *= 0.5f;
		// 	value = (1.0f - value) * 0.9f;
		// 	_filterSmooth->SetBlendParams(1, value);
		// }

        if (_filterSmooth){
            _filterSmooth->SetLightenFactor(value);
        }
#endif        
	}

	void KuGouEffectImpl::SetBeautySoftlightFactor(float value)
	{
#if MODULE_BEAUTY         
		// if (_filterSmooth) {
		// 	value *= 0.5f;
		// 	_filterSmooth->SetBlendParams(2, value);
		// }

        if (_filterSmooth){
            _filterSmooth->SetSoftlightFactor(value);
        }
#endif        
	}

	void KuGouEffectImpl::SetBeautySaturateFactor(float value)
	{
#if MODULE_BEAUTY         
		// if (_filterSmooth) {
		// 	value *= 0.3f;
		// 	_filterSmooth->SetBlendParams(3, value);
		// }

        if (_filterSmooth){
            _filterSmooth->SetSaturateFactor(value);
        }
#endif        
	}

    void KuGouEffectImpl::SetBeautySharpenFactor(float value)
    {
#if MODULE_BEAUTY         
		 if (_filterSharpen) {
		 	_filterSharpen->SetSharpness(value);
		 }

        // if (_filterSharpen2) {
        //     _filterSharpen2->SetSharpness(value);
        // }
#endif
    }

    void KuGouEffectImpl::SetBeautyBlurFactor(float value)
    {
#if MODULE_BEAUTY    
        if (_filterBlur){
            _filterBlur->SetThresholdFactor(value);
        }
#endif
    }

    void KuGouEffectImpl::SetBeautyLevel(int level)
    {
        assert(level >= 0);
        if (level < 0) return;

        SetBeautyEnable(level > 0);

        switch(level)
        {
        case 1:
            SetBeautyBlurFactor(0.45f);
            SetBeautySmoothFactor(0.35f);
            SetBeautyWhiteFactor(0.1f);
            SetBeautySoftlightFactor(0.55f);
            SetBeautySaturateFactor(0.75);
            SetBeautySharpenFactor(0.3f);
            break;
        case 2:
            SetBeautyBlurFactor(0.45f);
            SetBeautySmoothFactor(0.55f);
            SetBeautyWhiteFactor(0.2f);
			SetBeautySoftlightFactor(0.35f);
            SetBeautySaturateFactor(0.65);
            SetBeautySharpenFactor(0.3f);
            break;
        case 3:
            SetBeautyBlurFactor(0.65f);
            SetBeautySmoothFactor(0.75f);
            SetBeautyWhiteFactor(0.3f);
            SetBeautySoftlightFactor(0.3f);
            SetBeautySaturateFactor(0.4f);
            SetBeautySharpenFactor(0.3f);
            break;
        case 4:
            SetBeautyBlurFactor(0.85f);
            SetBeautySmoothFactor(0.75f);
            SetBeautyWhiteFactor(0.35f);
            SetBeautySoftlightFactor(0.1f);
            SetBeautySaturateFactor(0.2f);
            SetBeautySharpenFactor(0.35f);
            break;
        default:
            if (level != 0){
                LOGW("set beauty level param invalid!, level = %d", level);
            }
            break;
        }
    }

    void KuGouEffectImpl::SetVideoEnable(bool enable)
    {
        if (enable){
            _effectState |= kBeautyEnableVideo;
        }else{
            _effectState &= ~kBeautyEnableVideo;
        }
        _needRejoin = true;
    }    

    void KuGouEffectImpl::SetStyleEnable(bool enable)
    {
#if MODULE_STYLE 
        if (enable){
            _effectState |= kBeautyEnableStyle;
        }else{
            _effectState &= ~kBeautyEnableStyle;
        }
        _needRejoin = true;
#endif
    }

    void KuGouEffectImpl::SetStyleType(int index)
    {
#if MODULE_STYLE         
        if (_filterStyle && _styleSamples)
        {
            int count = 0;
            for (int c=0; c<3; c++)
            {
                double* points = 0;
                _styleSamples->GetCurvePoints(index, c, &points, &count);
                _filterStyle->SetSamplePoints(c, points, count);
            }
        }
#endif        
    }

    void KuGouEffectImpl::SetTitleEnable(bool enable)
    {
#if MODULE_TITLE        
        if (enable){
            _effectState |= kBeautyEnableTitle;
        }else{
            _effectState &= ~kBeautyEnableTitle;
        }
        _needRejoin = true;
#endif
    }

    void KuGouEffectImpl::SetTitleImage(void* data, int width, int height, int stride, int bitCount)
    {
#if MODULE_TITLE        
        if (_filterTitle){
            _filterTitle->SetTitleImage(data, width, height, stride, bitCount);
        }
#endif
    }

    void KuGouEffectImpl::SetFaceAREnable(bool enable)
    {
#if MODULE_FACEAR
        if (enable){
            _effectState |= kBeautyEnableFaceAR;
        }else{
            _effectState &= ~kBeautyEnableFaceAR;
        }
        _needRejoin = true;
#endif	
    }

    void KuGouEffectImpl::SetBeautyEnable(bool enable)
    {
#if MODULE_BEAUTY        
        bool lastState = _effectState & kBeautyEnableBeauty;
        if (enable){
            _effectState |= kBeautyEnableBeauty;
        }else{
            _effectState &= ~kBeautyEnableBeauty;
        }
        _needRejoin = (lastState != enable);
#endif
    }   

    void KuGouEffectImpl::SetDisplayEnable(bool enable)
    {
#if MODULE_DISPLAY        
        if (enable){
            _effectState |= kBeautyEnableDisplay;
        }else{
            _effectState &= ~kBeautyEnableDisplay;
        }
        _needRejoin = true;
#endif
    } 

    void KuGouEffectImpl::SetOutputEnable(bool enable)
    {
#if MOUDEL_OUTPUT
        if (enable){
            _effectState |= kBeautyEnableOutput;
        }else{
            _effectState &= ~kBeautyEnableOutput;
        }
        _needRejoin = true;
#endif
    }

    GPUImageOutput* KuGouEffectImpl::JoinFaceARFilters(GPUImageOutput* filterParent)
    {
#if MODULE_FACEAR
        filterParent->AddTarget(_filterSmall);
        filterParent->AddTarget(_filterAnimate);
        filterParent->AddTarget(_filterFaceAR);   //filterCamera必须作为filterFaceU的第一个输入，顺序不能反了
        _filterAnimate->AddTarget(_filterFaceAR); //filterAnimate必须作为filterFaceU的第二个输入，顺序不能反了
        return _filterFaceAR;
#else
        return filterParent;
#endif
    }

    void KuGouEffectImpl::BreakFaceARFilters(GPUImageOutput* filterParent)
    {
#if MODULE_FACEAR
        filterParent->RemoveTarget(_filterSmall);
        filterParent->RemoveTarget(_filterAnimate);
        filterParent->RemoveTarget(_filterFaceAR);
        _filterFaceAR->RemoveAllTargets();
#endif
    }

    GPUImageOutput* KuGouEffectImpl::JoinBeautyFilters(GPUImageOutput* filterParent)
    {
#if 1
		 filterParent->AddTarget(_filterBlur);
		 filterParent->AddTarget(_filterSmooth);	//filterParent必须作为filterSmooth的第一个输入
		 _filterBlur->AddTarget(_filterSmooth); 	//filterBlur必须作为filterSmooth的第二个输入
		 _filterSmooth->AddTarget(_filterSharpen);
		 return _filterSharpen;

//		filterParent->AddTarget(_filterBlur);
//		filterParent->AddTarget(_filterSmooth);	//filterParent必须作为filterSmooth的第一个输入
//		_filterBlur->AddTarget(_filterSmooth); 	//filterBlur必须作为filterSmooth的第二个输入
//
//		filterParent->AddTarget(_filterHighpass);
//		_filterHighpass->AddTarget(_filterSharpen2);
//		_filterSmooth->AddTarget(_filterSharpen2);
//		return _filterSharpen2;
#else
//		filterParent->AddTarget(_filterBlur);
//		return _filterBlur;
//		filterParent->AddTarget(_filterHighpass);
//		return _filterHighpass;
#endif
    }

    void KuGouEffectImpl::BreakBeautyFilters(GPUImageOutput* filterParent)
    {
#if MODULE_BEAUTY
        filterParent->RemoveTarget(_filterBlur);
        filterParent->RemoveTarget(_filterSmooth);
        _filterSmooth->RemoveTarget(_filterSharpen);
        _filterSharpen->RemoveAllTargets();
#endif
    }

    GPUImageOutput* KuGouEffectImpl::JoinStyleFilters(GPUImageOutput* filterParent)
    {
#if MODULE_STYLE
        filterParent->AddTarget(_filterStyle);
        return _filterStyle;
#else
        return filterParent;        
#endif
    }

    void KuGouEffectImpl::BreakStyleFilters(GPUImageOutput* filterParent)
    {
#if MODULE_STYLE
        filterParent->RemoveTarget(_filterStyle);
#endif
    }

    GPUImageOutput* KuGouEffectImpl::JoinTitleFilters(GPUImageOutput* filterParent)
    {
#if MODULE_TITLE
        filterParent->AddTarget(_filterTitle);
        return _filterTitle;
#else
        return filterParent;
#endif
    }

    void KuGouEffectImpl::BreakTitleFilters(GPUImageOutput* filterParent)
    {
#if MODULE_TITLE
        filterParent->RemoveTarget(_filterTitle);
#endif        
    }

    void KuGouEffectImpl::JoinFilters(void)
    {
        GPUImageOutput* lastFilter = 0;

        if (!(_effectState & kBeautyEnableVideo)){
            lastFilter = _filterCamera;
            LOGD("join camera filter");
        }else{
            lastFilter = _filterVideo;
            LOGD("join video filter");
        }

#if MODULE_FACEAR
        if (_effectState & kBeautyEnableFaceAR) {
            lastFilter = JoinFaceARFilters(lastFilter);
            LOGD("join facear filter");
        }else{
            //BreakFaceARFilters(lastFilter);
        }
#endif

#if MODULE_BEAUTY
        if (_effectState & kBeautyEnableBeauty) {
            lastFilter = JoinBeautyFilters(lastFilter);
            LOGD("join beauty filter");
        }else{
            //BreakBeautyFilters(lastFilter);
        }
#endif

#if MODULE_STYLE
        if (_effectState & kBeautyEnableStyle){
            lastFilter = JoinStyleFilters(lastFilter);
            LOGD("join style filter");
        }else{
            //BreakBeautyFilters(lastFilter);
        }
#endif

#if MODULE_TITLE
        if (_effectState & kBeautyEnableTitle){
            lastFilter = JoinTitleFilters(lastFilter);
            LOGD("join title filter");
        }else{
            //BreakTitleFilters(lastFilter);
        }
#endif

#if MODULE_DISPLAY
        if (_effectState & kBeautyEnableDisplay){
            lastFilter->AddTarget(_filterDisplay);
            LOGD("join display filter");
        }
#endif

#if MODULE_OUTPUT
		if (_effectState & kBeautyEnableOutput){
            lastFilter->AddTarget(_filterWriter);
            LOGD("join output filter");
        }
#endif
		_needRejoin = false;
    }    

    void KuGouEffectImpl::BreakFilters()
    {
        _filterVideo->RemoveAllTargets();
        _filterCamera->RemoveAllTargets();
#if MODULE_FACEAR
        _filterSmall->RemoveAllTarget();
        _filterAnimate->RemoveAllTarget();
        _filterFaceAR->RemoveAllTarget();
#endif

#if MODULE_BEAUTY
        _filterBlur->RemoveAllTargets();
        _filterSmooth->RemoveAllTargets();
        _filterSharpen->RemoveAllTargets();
#endif

#if MODULE_STYLE
        _filterStyle->RemoveAllTargets();
#endif

#if MODULE_TITLE
        _filterTitle->RemoveAllTargets();
#endif
    }

    //render
    void KuGouEffectImpl::Render(void* data, int width, int height,  int format)
    {
        if (_needRejoin)
        {
            BreakFilters();
            JoinFilters();
        }

        //native video file
        if (_effectState & kBeautyEnableVideo)
        {
            if (_filterVideo){
                _filterVideo->RenderToTexture(data, width, height, format);
            }
        }
        else//camera device
        {
            if (_filterCamera) {
                _filterCamera->RenderToTexture(data, width, height, format);
            }
        }
    }

    void KuGouEffectImpl::Render(void* dst, void* src, int width, int height,  int format)
    {
        if (_needRejoin)
		{
            BreakFilters();
            JoinFilters();
        }

        if (_filterWriter)
		{
            _filterWriter->SetOutputCallback((char*)dst);
        }

        //native video file
        if (_effectState & kBeautyEnableVideo)
		{
            if (_filterVideo){
                _filterVideo->RenderToTexture(src, width, height, format);
            }
        }
		else
		{
            if (_filterCamera) {
                _filterCamera->RenderToTexture(src, width, height, format);
            }
        }
    }
}