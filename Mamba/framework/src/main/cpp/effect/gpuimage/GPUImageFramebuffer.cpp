//
// Created by yongfali on 2016/3/22.
//

#include "include/GPUImageFramebuffer.h"
#include "include/GPUImageContext.h"

namespace e
{
    GPUImageTextureOptions kGPUImageTextureOptionsDefault(GL_LINEAR,GL_LINEAR, GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE, GL_RGBA,GL_RGBA, GL_UNSIGNED_BYTE);
    GPUImageTextureOptions kGPUImageTextureOptionsLuminance(GL_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE);
    GPUImageTextureOptions kGPUImageTextureOptionsChrominance(GL_LINEAR,GL_LINEAR,GL_CLAMP_TO_EDGE,GL_CLAMP_TO_EDGE, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE);

    static key_t GenHashKey(Size size, GPUImageTextureOptions & textureOptions, bool onlyTexture)
    {
        char text[256] = {0};
        if (onlyTexture)
        {
            sprintf(text, "%d-%d:%d:%d:%d:%d:%d:%d:%d-NOFBO"
                , size.width
                , size.height
                , textureOptions.minFilter
                , textureOptions.magFilter
                , textureOptions.wrapS
                , textureOptions.wrapT
                , textureOptions.internalFormat
                , textureOptions.format
                , textureOptions.type);
        }
        else
        {
            sprintf(text, "%d-%d:%d:%d:%d:%d:%d:%d:%d-FBO"
                , size.width
                , size.height
                , textureOptions.minFilter
                , textureOptions.magFilter
                , textureOptions.wrapS
                , textureOptions.wrapT
                , textureOptions.internalFormat
                , textureOptions.format
                , textureOptions.type);
        }

        return GenHashKey(text, strlen(text));
    }
    
    GPUImageFramebuffer::GPUImageFramebuffer(Size size)
    {
        this->eglContext = eglGetCurrentContext();
        this->texture = 0;
        this->frameBuffer = 0;
        this->onlyTexture = false;
        this->hashKey = 0;

        SetTextureOptionsDefault(textureOptions);
        
        Create(size, textureOptions, false);
    }
    
    GPUImageFramebuffer::GPUImageFramebuffer(Size size
        , GPUImageTextureOptions & textureOptions
        , bool onlyTexture)
    {
        this->eglContext = eglGetCurrentContext();
        this->texture = 0;
        this->frameBuffer = 0;
        this->onlyTexture = false;
        this->hashKey = 0;

        SetTextureOptionsDefault(textureOptions);
        
        Create(size, textureOptions, onlyTexture);
    }
    
    GPUImageFramebuffer::GPUImageFramebuffer(Size size, GLuint texture)
    {
        this->eglContext = eglGetCurrentContext();
        this->size = size;
        this->texture = texture;
        this->frameBuffer = 0;
        this->onlyTexture = false;
        this->hashKey = 0;

        SetTextureOptionsDefault(textureOptions);
        
        Create(size, textureOptions, onlyTexture);
    }
    
    GPUImageFramebuffer::~GPUImageFramebuffer(void)
	{
/*		关于context丢失的官网描述
 *
		EGL Context Lost

		There are situations where the EGL rendering context will be lost.
 		This typically happens when device wakes up after going to sleep.
 		When the EGL context is lost, all OpenGL resources (such as textures)
 		that are associated with that context will be automatically deleted.
 		In order to keep rendering correctly, a renderer must recreate any
 		lost resources that it still needs. The onSurfaceCreated(GL10, EGLConfig) 
 		method is a convenient place to do this.
*/
		if (eglContext != eglGetCurrentContext())
		{
			LOGD("GPUImageFramebuffer current context not equal local!");
			return;
		}

        if (texture)
        {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
        
        if (frameBuffer)
        {
            glDeleteFramebuffers(1, &frameBuffer);
            frameBuffer = 0;
        }
    }
    
    void GPUImageFramebuffer::GenTexture(void)
    {
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureOptions.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureOptions.magFilter);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureOptions.wrapS);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureOptions.wrapT);

        glTexImage2D(GL_TEXTURE_2D
            , 0
            , textureOptions.internalFormat
            , size.width
            , size.height
            , 0
            , textureOptions.format
            , textureOptions.type
            , 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void GPUImageFramebuffer::GenFramebuffer(void)
    {
        //generate framebuffer
        glGenFramebuffers(1, &frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        //generate texture
        if (!texture) GenTexture();
        //bind texture to framebuffer
        glBindTexture(GL_TEXTURE_2D, texture);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
        //check
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE){
            LOGE("check framebuffer failed: 0x%08x", status);
        }else{
            //LOGD("create framebuffer ok: %d x %d", _size.width, _size.height);
        }
        
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void GPUImageFramebuffer::Active(void)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glViewport(0, 0, size.width, size.height);
    }
    
    bool GPUImageFramebuffer::Create(Size size, GPUImageTextureOptions & textureOptions, bool onlyTexture)
    {
        this->size = size;
        this->onlyTexture = onlyTexture;
        this->textureOptions = textureOptions;
        this->hashKey = GenHashKey(size, textureOptions, onlyTexture);
            
        if (onlyTexture){
            GenTexture();
            frameBuffer = 0;
        }else{
            GenFramebuffer();
        }
        
        return true;
    }
    
    GLuint GPUImageFramebuffer::GetTexture(void) const 
    {
        return texture;
    }
    
    GLuint GPUImageFramebuffer::GetFramebuffer(void) const 
    {
        return frameBuffer;
    }   

    key_t GPUImageFramebuffer::GetHashKey(void) const
    {
        return hashKey;
    }

    void GPUImageFramebuffer::DeleteObject(void)
    {
		GPUImageFramebufferCache::Singleton()->ReleaseFramebuffer(this);
    }
    
    //-------------------------------------------------------------------------------
    GPUImageFramebufferCache* GPUImageFramebufferCache::instance = NULL;
    GPUImageFramebufferCache::GPUImageFramebufferCache(void)
    {
        allocCount = 0;
        frameBuffers = new GPUImageFramebufferList();
        assert(frameBuffers);
    }
    
    GPUImageFramebufferCache::~GPUImageFramebufferCache(void)
    {
        GPUImageFramebufferList::iterator it = frameBuffers->begin();
        for (; it!=frameBuffers->end(); it++)
        {
            GPUImageFramebuffer* fbo = *it;
            SafeDelete(&fbo);
            allocCount--;
        }
        if (allocCount > 0){
            LOGW("GPUImage framebuffer memory leak!!! :%d", allocCount);
        }else{
            LOGD("GPUImage framebuffer release success, %d", allocCount);
        }
        SafeDelete(&frameBuffers);
		instance = 0;
    }

    GPUImageFramebufferCache* GPUImageFramebufferCache::Singleton(void)
    {
        if (instance == NULL)
		{
            instance = new GPUImageFramebufferCache();
        }
        return instance;
    }

    void GPUImageFramebufferCache::ReleaseFramebuffer(GPUImageFramebuffer* frameBuffer)
    {
        if (frameBuffer){
            frameBuffers->push_back(frameBuffer);
        }
    }
    
    GPUImageFramebuffer* GPUImageFramebufferCache::FetchFramebuffer(Size size, bool onlyTexture) 
    {
        return FetchFramebuffer(size, kGPUImageTextureOptionsDefault, onlyTexture);
    }

    GPUImageFramebuffer* GPUImageFramebufferCache::FetchFramebuffer(Size size
        , GPUImageTextureOptions & textureOptions
        , bool onlyTexture)
    {
        key_t key = GenHashKey(size, textureOptions, onlyTexture);

        GPUImageFramebufferList::iterator it = frameBuffers->begin();
        for (; it != frameBuffers->end(); it++)
        {
            GPUImageFramebuffer* fbo = (*it);
            if (fbo->GetHashKey() == key)
            {
                frameBuffers->erase(it);
                return fbo;
            }
        }
        //new an framebuffer object
        LOGD("GPUImageFramebufferCache lookup no hit, key = 0x%08zd, %d x %d, onlyTexture = %s",
            key, size.width, size.height, onlyTexture?"true":"false");
        
        GPUImageFramebuffer* frameBuffer = new GPUImageFramebuffer(size, textureOptions, onlyTexture);
        if (frameBuffer != NULL) allocCount++;
        
        return frameBuffer;
    }
}
