//
// Created by yongfali on 2017/3/17.
//

#ifndef KUGOUEFFECT_GPUIMAGEVIDEOFILTER_H
#define KUGOUEFFECT_GPUIMAGEVIDEOFILTER_H

#include "GPUImageBase.h"
#include "GPUImageOutput.h"

namespace e
{
#ifndef SHADER_STRING
#	define SHADER_STRING(x) #x
#endif

	extern shader_t kGPUImageVideoVertexShaderString;
	extern shader_t kGPUImageVideoFragmentShaderString;

    class GPUImageVideoFilter : public GPUImageOutput
    {
    public:
        DEFINE_CREATE(GPUImageVideoFilter)
    CONSTRUCTOR_ACCES:    
        GPUImageVideoFilter();
        ~GPUImageVideoFilter();
    public:
        //set it if do scale
		virtual void SetOutputSize(Size size);
		//rendering
		virtual void SetInputSize(Size size, int index);
		virtual void SetInputRotationMode(GPUImageRotationMode & rotationMode, int index);
		virtual void RenderToTexture(void* data, int width, int height, int format);
		//init
		virtual bool Initialize(void);
		virtual bool Initialize(const char *vShaderString, const char* fShaderString);
	protected:
		virtual void InitializeAttributes(void);
		virtual Size GetSizeOfFBO(void);
		virtual Size GetSizeOfRotated(Size size);

		bool CreateTextures(void);
		void UpdateTextures(unsigned char* data, int width, int height, int format);
		void ConvertYUV2RGBAOutput(void);
		void CallNextTargets(Time time);
		//此函数只能用于这个类中
		GLfloat* GetTexCoords(const GPUImageRotationMode & rotateMode);      
    protected:
        EGLContext _eglContext;
		Size _inputSize;
		Size _outputSize;
		Ptr<GLProgram> _filterProgram;
        Ptr<GPUImageFramebufferCache> _frameBufferCache;
#ifndef TEXTURE_CACHE_ENABLE
		EGLContext _eglContext;
		GLuint _luminanceTexture;
		GLuint _chrominanceTexture;
#else
        Ptr<GPUImageTexture> _luminanceTexture;
        Ptr<GPUImageTexture> _chrominanceTexture;
        Ptr<GPUImageTextureCache> _textureCache;
#endif
    	GLuint _filterPositionAttribute;
    	GLuint _filterTextureCoordinateAttribute;
    	GLint _luminanceTextureUniform;
    	GLint _chrominanceTextureUniform;
    	GLint _conversionMatrixUniform;
	
		GPUImageRotationMode _inputRotation;
		GLfloat _conversionMatrix[9];
    };
}

#endif //KUGOUEFFECT_GPUIMAGEVIDEOFILTER_H
