//
// Created by yongfali on 2016/4/7.
//

#include "include/KuGouEffect.h"
#include "GPUImageBase.h"
#include "include/KuGouEffectImpl.h"

namespace e
{
	const char* g_licence = "KuGou-Media-Group";
    KuGouEffect* KuGouEffect::singleton = 0;
    KuGouEffect* KuGouEffect::Singleton(KuGouEffectProfile profile, const char* licence)
    {
        if (strcmp(licence, g_licence))
        {
            LOGD("KuGouEffect licence invalid!!!");
            return NULL;
        }
        
        if (singleton == 0){
            singleton = new KuGouEffect();
            assert(singletion != 0);
        }
        return singleton;
    }

    void KuGouEffect::ReleaseInstance()
    {
        if (singleton != 0){
            delete singleton;
            singleton = 0;
        }
    }

    KuGouEffect::KuGouEffect(void)
    {
        impl = KuGouEffectImpl::Singleton();
    }

    KuGouEffect::~KuGouEffect(void)
    {
        impl = 0;
        KuGouEffectImpl::ReleaseInstance();
    }

	bool KuGouEffect::IsValid() const
	{
		if (impl){
			return impl->IsValid();
		}
		return false;
	}

	void KuGouEffect::SetViewport(int x, int y, int width, int height)
    {
		if (impl){
			impl->SetViewport(x, y, width, height);
		}
	}

    void KuGouEffect::SetImageRotation(int angle, int flipX, int flipY)
    {
        if (impl) {
            impl->SetImageRotation(angle, flipX, flipY);
        }
    }

	void KuGouEffect::SetOutputSize(int width, int height)
	{
		if (impl) {
			impl->SetOutputSize(width, height);
		}
	}

    void KuGouEffect::SetDeviceRotation(int angle)
    {
        if (impl) {
            impl->SetDeviceRotation(angle);
        }
    }

    void KuGouEffect::SetAnimateCacheSize(int size)
    {
        if (impl) {
            impl->SetAnimateCacheSize(size);
        }
    }
    
    void KuGouEffect::SetAnimatePath(const char* path)
    {
        if (impl) {
            impl->SetAnimatePath(path);
        }
    }

    void KuGouEffect::SetFaceAREnable(bool enable)
    {
        if (impl) {
            impl->SetFaceAREnable(enable);
        }
    }

	void KuGouEffect::SetFaceARNotify(KuGouEffectCallback callback)
	{
        //TODO:
	}

    void KuGouEffect::SetBeautyEnable(bool enable)
    {
        if (impl) {
            impl->SetBeautyEnable(enable);
        }
    }

    void KuGouEffect::SetBeautyParams(BeautyParams type, float value)
    {
        switch(type)
        {
			case kBeautySmoothFactor:
				if (impl) impl->SetBeautySmoothFactor(value);
				break;
			case kBeautyWhiteFactor:
				if (impl) impl->SetBeautyWhiteFactor(value);
				break;
			case kBeautySoftlyFactor:
				if (impl) impl->SetBeautySoftlightFactor(value);
				break;
			case kBeautyPinklyFactor:
				if (impl) impl->SetBeautySaturateFactor(value);
				break;
			case kBeautySharpenFactor:
				if (impl) impl->SetBeautySharpenFactor(value);
				break;
			default:
				break;
		}
    }

    void KuGouEffect::SetBeautyLevel(int level)
    {
        if (impl){
            impl->SetBeautyLevel(level);
        }
    }

    void KuGouEffect::SetStyleEnable(bool enable)
    {
        if (impl){
            impl->SetStyleEnable(enable);
        }
    }

    void KuGouEffect::SetStyleType(int index)
    {
        if (impl){
            impl->SetStyleType(index);
        }
    }

    void KuGouEffect::SetTitleEnable(bool enable)
    {
        if (impl){
            impl->SetTitleEnable(enable);
        }
    }

    void KuGouEffect::SetTitleImage(void* data, int width, int height, int stride, int bitCount)
    {
        if (impl){
            impl->SetTitleImage(data, width, height, stride, bitCount);
        }
    }

    void KuGouEffect::SetDisplayEnable(bool enable)
    {
        if (impl){
            impl->SetDisplayEnable(enable);
        }
    }

    void KuGouEffect::SetVideoEnable(bool enable)
    {
        if (impl){
            impl->SetVideoEnable(enable);
        }
    }

	void KuGouEffect::Render(void* data, int width, int height, int format)
	{
#ifdef _DEBUG
		if (impl){
			impl->Render(data, width, height, format);
		}
#else
		static uint64_t spanTime = 0, lastTime = 0, frameCount = 0;
		if (impl) {
            uint64_t time = Time::GetTime();
            impl->Render(data, width, height, format);
            spanTime += Time::GetTime() - time;
        }

        //fps calculate
        frameCount++;
        uint64_t dt = Time::GetTime() - lastTime;
        if (dt >= 1000)
        {
            int fps = frameCount;
            int avg = spanTime / frameCount;
            spanTime = 0;
			lastTime += dt;
			frameCount = 0;
            LOGD("KuGouEffect render %d x %d, %d(fps), %d(ms)", width, height, fps, avg);
        }
#endif
	}

    void KuGouEffect::Render(void* dst, void* src, int width, int height, int format)
    {
#ifdef _DEBUG
		if (impl){
			impl->Render(dst, src, width, height, format);
		}
#else
		static uint64_t spanTime = 0, lastTime = 0, frameCount = 0;
		if (impl) {
            uint64_t time = Time::GetTime();
            impl->Render(dst, src, width, height, format);
            spanTime += Time::GetTime() - time;
        }

        //fps calculate
        frameCount++;
        uint64_t dt = Time::GetTime() - lastTime;
        if (dt >= 1000)
        {
            int fps = frameCount;
            int avg = spanTime / frameCount;
            spanTime = 0;
			lastTime += dt;
			frameCount = 0;
            LOGD("KuGouEffect render %d x %d, %d(fps), %d(ms)", width, height, fps, avg);
        }
#endif
    }
}