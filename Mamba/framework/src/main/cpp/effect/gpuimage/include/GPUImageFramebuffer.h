//
// Created by yongfali on 2016/3/22.
//

#ifndef E_GPUIMAGE_FRAMEBUFFER_H
#define E_GPUIMAGE_FRAMEBUFFER_H

#include "GPUImageTextureOptions.h"

namespace e
{
    extern GPUImageTextureOptions kGPUImageTextureOptionsDefault;
    extern GPUImageTextureOptions kGPUImageTextureOptionsLuminance;
    extern GPUImageTextureOptions kGPUImageTextureOptionsChrominance;

    //framebuffer class 
    class GPUImageFramebuffer : public RefObject
    {
    public:
        GPUImageFramebuffer(Size size);
        GPUImageFramebuffer(Size size, GPUImageTextureOptions & textureOptions, bool onlyTexture);
        GPUImageFramebuffer(Size size, GLuint texture);
        virtual ~GPUImageFramebuffer(void);
    public:
        void Active(void);
        GLuint GetTexture(void) const;
        GLuint GetFramebuffer(void) const;
        key_t GetHashKey(void) const;
    protected:
        void GenTexture(void);
        void GenFramebuffer(void);    
        bool Create(Size size, GPUImageTextureOptions & textureOptions, bool onlyTexture);
        virtual void DeleteObject(void);
    protected:
        EGLConfig eglContext;
        Size size;
        GLuint texture;
        GLuint frameBuffer;
        bool onlyTexture;
        GPUImageTextureOptions textureOptions;
        key_t hashKey;
    };

    typedef Ptr<GPUImageFramebuffer> GPUImageFramebufferPtr;
   
    //frame buffer cache
    class GPUImageFramebufferCache : public RefObject
    {
    public:
        GPUImageFramebufferCache(void);
        virtual ~GPUImageFramebufferCache(void);
    public:      
        GPUImageFramebuffer* FetchFramebuffer(Size size, bool onlyTexture);//use default texture options
        GPUImageFramebuffer* FetchFramebuffer(Size size, GPUImageTextureOptions & textureOptions, bool onlyTexture);
        void ReleaseFramebuffer(GPUImageFramebuffer* frameBuffer); 
        static GPUImageFramebufferCache* Singleton(void);
    protected:
        static GPUImageFramebufferCache* instance;
        typedef std::list<GPUImageFramebuffer*> GPUImageFramebufferList;
        GPUImageFramebufferList* frameBuffers;
        int allocCount;
    };

    typedef Ptr<GPUImageFramebufferCache> GPUImageFramebufferCachePtr;
}

#endif //__E_GPUIMAGE_FRAMEBUFFER_H__
