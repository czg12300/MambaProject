//
// Created by liyongfa on 2016/11/16.
//

#ifndef E_GPUIMAGETEXTURE_H
#define E_GPUIMAGETEXTURE_H

#include "GPUImageBase.h"
#include "GPUImageTextureOptions.h"

namespace e
{
    class GPUImageTexture : public RefObject
    {
    public:
        GPUImageTexture(Size size, GPUImageTextureOptions & options, int index);
        virtual ~GPUImageTexture();
    public:
        GLuint GetTexture() const;
        Size GetSize() const;
        key_t GetKey() const;
    protected:
        bool Create(Size size, GPUImageTextureOptions & options, int index);
        virtual void DeleteObject();
    protected:
        EGLConfig eglContext;
        Size size;
        GLuint native;
        GPUImageTextureOptions options;
        key_t hashKey;
    };

    typedef Ptr<GPUImageTexture> GPUImageTexturePtr;

    //texture cache
    class GPUImageTextureCache : public RefObject
    {
    public:
        GPUImageTextureCache(void);
        virtual ~GPUImageTextureCache(void);
    public:
        GPUImageTexture* Fetch(Size size, GPUImageTextureOptions & options, int index);
        void Release(GPUImageTexture* object);
        static GPUImageTextureCache* Singleton(void);
    protected:
        typedef std::list<GPUImageTexture*> GPUImageTextureList;
        GPUImageTextureList* textureList;
        int allocCount;
    protected:
        static GPUImageTextureCache* instance;
    };

    typedef Ptr<GPUImageTextureCache> GPUImageTextureCachePtr;
}
#endif //E_GPUIMAGETEXTURE_H
