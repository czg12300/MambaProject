//
// Created by yongfali on 2016/4/7.
//

#ifndef E_KUGOUEFFECT_H
#define E_KUGOUEFFECT_H
#include "KuGouEffectCommon.h"

namespace e
{
    class KuGouEffectImpl;
    class KuGouEffect
    {
    private:
        KuGouEffect(void);
        virtual ~KuGouEffect(void);
        bool IsValid() const;
    public:
        static KuGouEffect* Singleton(KuGouEffectProfile profile, const char* licence);
        static void ReleaseInstance(void);
    public:
		void SetViewport(int x, int y, int width, int height);
		void SetDeviceRotation(int angle);
        void SetImageRotation(int angle, int flipX, int flipY);
		void SetOutputSize(int width, int height);
		void SetVideoEnable(bool enable);
		void SetFaceAREnable(bool enable);
		void SetBeautyEnable(bool enable);
		void SetStyleEnable(bool enable);
		void SetTitleEnable(bool enable);
		void SetDisplayEnable(bool enable);
        void SetAnimateCacheSize(const int size);
        void SetAnimatePath(const char* path);
        void SetFaceARNotify(KuGouEffectCallback callback);
        void SetBeautyParams(BeautyParams type, float value);
        void SetBeautyLevel(int level);
        void SetStyleType(int index);
        void SetTitleImage(void* data, int width, int height, int stride, int bitCount);
        void Render(void* data, int width, int height, int format);
        void Render(void* dst, void* src, int width, int height, int format);
    protected:
        KuGouEffectImpl* impl;
        static KuGouEffect* singleton;
    };
}

#endif //E_KUGOUEFFECT_H
