//
// Created by yongfali on 2017/3/17.
//

#ifndef KUGOUEFFECT_GPUIMAGETITLEFILTER_H
#define KUGOUEFFECT_GPUIMAGETITLEFILTER_H

#include "GPUImageBase.h"
#include "GPUImageFilter.h"

namespace e
{
    class GPUImageTitleFilter : public GPUImageFilter
    {
    public:
        DEFINE_CREATE(GPUImageTitleFilter)
    CONSTRUCTOR_ACCES :
        GPUImageTitleFilter();
        ~GPUImageTitleFilter();
    public:
        virtual bool Initialize();
        virtual void InitializeAttributes(void);
        void SetTitleImage(void* data, int width, int height, int stride, int bitCount);
    protected:
        void AllocateBuffer(int width, int height);
        void UpdateTitleTexture();
        void MakeTestImage();
        virtual void RenderToTexture(const float* vertices, const float* texCoords);
    protected:
        GLuint _filterTextureCoordinateAttribute2;
#ifndef TEXTURE_CACHE_ENABLE
        EGLContext _eglContext;
        GLuint _titleTexture;
#else
        Ptr<GPUImageTexture> _titleTexture;
        Ptr<GPUImageTextureCache> _textureCache;
#endif
        GLuint _titleTextureUniform;

        char* _titleImage;
        int _titleSize;
        int _titleWidth;
        int _titleHeight;
        int _titleStride;
        int _titleBitCount;
        bool _isNeedUpdate;
    };
}

#endif //KUGOUEFFECT_GPUIMAGETITLEFILTER_H
