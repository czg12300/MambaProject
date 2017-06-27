//
// Created by yongfali on 2016/4/9.
//

#ifndef E_GPUIMAGE_DISPLAYFILTER_H
#define E_GPUIMAGE_DISPLAYFILTER_H

#include "GPUImageBase.h"
#include "GPUImageContext.h"
#include "GPUImageFramebuffer.h"
#include "GLProgram.h"

namespace e
{
    extern shader_t kGPUImageDisplayVertexShaderString;
    extern shader_t kGPUImageDisplayFragmentShaderString;

	struct Viewport
	{
		int x, y, width, height;
	};
    
	class GPUImageDisplayFilter 
        : public RefObject
        , public GPUImageInput 
    {
	public:
        DEFINE_CREATE(GPUImageDisplayFilter)
    CONSTRUCTOR_ACCES:
	    GPUImageDisplayFilter(void);
	    virtual ~GPUImageDisplayFilter(void);
	public:
        virtual bool Initialize(void);
        virtual bool Initialize(const char* vShaderString, const char* fShaderString);
        virtual void InitializeAttributes(void);
		virtual void SetViewport(int x, int y, int width, int height);
        virtual void SetInputSize(Size size, int index);
        virtual void SetInputRotationMode(GPUImageRotationMode & rotationMode, int index);
        virtual void SetInputFramebuffer(Ptr<GPUImageFramebuffer> & frameBuffer, int index);
        virtual int NextAvailableTextureIndex(void);
        virtual void NewFrameReady(Time frameTime, int textureIndex);
        virtual void EndProcessing(void);
        virtual bool IsEnable(void);
        virtual void Reset(void);
    protected:
		Viewport _vp;
    	Size _inputTextureSize;
        Ptr<GLProgram> _filterProgram;
        GPUImageRotationMode _inputRotation;
    	GPUImageFramebufferPtr _inputFramebuffer;
        GLuint _filterPositionAttribute;
        GLuint _filterTextureCoordinateAttribute;
        GLint _filterInputTextureUniform;
	};

    typedef Ptr<GPUImageDisplayFilter> GPUImageDisplayFilterPtr;
}

#endif //_E_GPUIMAGE_DISPLAYFILTER_H_
