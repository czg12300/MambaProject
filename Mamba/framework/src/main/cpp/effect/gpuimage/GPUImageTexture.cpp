//
// Created by liyongfa on 2016/11/16.
//

#include "include/GPUImageTexture.h"

namespace e
{
    //////////////////////////////////////////////////////////////////////////////////////////////
    static key_t GenHashKey(Size size, GPUImageTextureOptions & options)
    {
        char text[256] = {0};
        sprintf(text, "%d-%d:%d:%d:%d:%d:%d:%d:%d-tex"
            , size.width
            , size.height
            , options.minFilter
            , options.magFilter
            , options.wrapS
            , options.wrapT
            , options.internalFormat
            , options.format
            , options.type);

        return GenHashKey(text, strlen(text));
    }

    GPUImageTexture::GPUImageTexture(Size size, GPUImageTextureOptions & options, int index)
        : eglContext(eglGetCurrentContext())
    {
        native = 0;
        hashKey = 0;

        Create(size, options, index);
    }

    GPUImageTexture::~GPUImageTexture()
    {
        if (eglContext != eglGetCurrentContext())
        {
			LOGW("GPUImageTexture current context not equal local!");
			return;
        }

        if (native != 0)
        {
            glDeleteTextures(1, &native);
            native = 0;
        }
    }

    bool GPUImageTexture::Create(Size size, GPUImageTextureOptions & options, int index)
    {
        this->size = size;
        this->options = options;
        this->hashKey = GenHashKey(size, options);

        glActiveTexture(GL_TEXTURE0 + index);
        glGenTextures(1, &native);
        glBindTexture(GL_TEXTURE_2D, native);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.magFilter);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrapS);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrapT);

        glTexImage2D(GL_TEXTURE_2D
            , 0
            , options.internalFormat
            , size.width
            , size.height
            , 0
            , options.format
            , options.type
            , 0);

        return native != 0;
    }

    GLuint GPUImageTexture::GetTexture() const
    {
        return native;
    }

    Size GPUImageTexture::GetSize() const
    {
        return size;
    }

    key_t GPUImageTexture::GetKey() const
    {
        return hashKey;
    }

    void GPUImageTexture::DeleteObject()
    {
        GPUImageTextureCache::Singleton()->Release(this);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    GPUImageTextureCache* GPUImageTextureCache::instance = NULL;
    GPUImageTextureCache::GPUImageTextureCache(void)
    {
        allocCount = 0;
        textureList = new GPUImageTextureList();
        assert(textureList);
    }

    GPUImageTextureCache::~GPUImageTextureCache(void)
    {
        GPUImageTextureList::iterator it = textureList->begin();
        for (; it != textureList->end(); it++)
        {
            GPUImageTexture* tex = *it;
            SafeDelete(&tex);
            allocCount--;
        }
        SafeDelete(&textureList);
        if (allocCount > 0){
            LOGW("GPUImageTexture memory leak!!! :%d", allocCount);
        }else {
            LOGD("GPUImageTexture release success, %d", allocCount);
        }
		instance = 0;
    }

    GPUImageTextureCache* GPUImageTextureCache::Singleton(void)
    {
        if (instance == NULL) {
            instance = new GPUImageTextureCache();
        }
        return instance;
    }

    void GPUImageTextureCache::Release(GPUImageTexture* object)
    {
        if (object != NULL){
            textureList->push_back(object);
        }
    }

    GPUImageTexture* GPUImageTextureCache::Fetch(Size size
        , GPUImageTextureOptions & options
        , int index)
    {
        key_t key = GenHashKey(size, options);
        GPUImageTextureList::iterator it = textureList->begin();
        for (; it != textureList->end(); it++)
        {
            GPUImageTexture* tex = (*it);
            if (tex->GetKey() == key)
            {
                textureList->erase(it);
                return tex;
            }
        }

        //new an framebuffer object
        LOGD("GPUImageTextureCache lookup no hit, key = 0x%08zd, %d x %d, %d",
             key, size.width, size.height, index);

        GPUImageTexture* texObject = new GPUImageTexture(size, options, index);
        if (texObject != NULL) {
            allocCount++;
        }

        return texObject;
    }
}
